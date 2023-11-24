// See LICENSE.Berkeley for license details.
// See LICENSE.SiFive for license details.

package freechips.rocketchip.rocket

import Chisel._
import Chisel.ImplicitConversions._
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.subsystem.CacheBlockBytes
import freechips.rocketchip.tile._
import freechips.rocketchip.tilelink._
import freechips.rocketchip.util._
import freechips.rocketchip.util.property._
import chisel3.internal.sourceinfo.SourceInfo
import chisel3.experimental._
import scala.collection.mutable.ListBuffer

class PTWReq(implicit p: Parameters) extends CoreBundle()(p) {
  val addr = UInt(width = vpnBits)
}

class PTWResp(implicit p: Parameters) extends CoreBundle()(p) {
  val ae = Bool()
  val pte = new PTE
  val level = UInt(width = log2Ceil(pgLevels))
  val fragmented_superpage = Bool()
  val homogeneous = Bool()
}
class TLBPTWIO(implicit p: Parameters) extends CoreBundle()(p)
    with HasCoreParameters {
  val req = Decoupled(Valid(new PTWReq))
  val resp = Valid(new PTWResp).flip
  val ptbr = new PTBR().asInput
  val status = new MStatus().asInput
  val pmp = Vec(nPMPs, new PMP).asInput
  val customCSRs = coreParams.customCSRs.asInput
  val vpoffset = Valid(new vpoffsetUpdate).flip
  
}

class PTWPerfEvents extends Bundle {
  val l2miss = Bool()
}

class DatapathPTWIO(implicit p: Parameters) extends CoreBundle()(p)
    with HasCoreParameters {
  val ptbr = new PTBR().asInput
  val sfence = Valid(new SFenceReq).flip
  val status = new MStatus().asInput
  val pmp = Vec(nPMPs, new PMP).asInput
  val perf = new PTWPerfEvents().asOutput
  val customCSRs = coreParams.customCSRs.asInput
  /*
   * priv-code-block
   */
  val pcode_req = Valid(new PCodeUpdate).flip
  val pcode_resp = Valid(new PCodeUpdate)
  val vpoffset_req = Valid(new vpoffsetUpdate).flip
  //val vpoffset_resp = Valid(new vpoffsetUpdate)
  //val pcode_debug = DecoupledIO(Bits(width=paddrBits))
 
}

class PTE(implicit p: Parameters) extends CoreBundle()(p) {
  val ppn = UInt(width = 54)
  val reserved_for_software = Bits(width = 2)
  val d = Bool()
  val a = Bool()
  val g = Bool()
  val u = Bool()
  val x = Bool()
  val w = Bool()
  val r = Bool()
  val v = Bool()

  def table(dummy: Int = 0) = v && !r && !w && !x
  def leaf(dummy: Int = 0) = v && (r || (x && !w)) && a
  def ur(dummy: Int = 0) = sr() && u
  def uw(dummy: Int = 0) = sw() && u
  def ux(dummy: Int = 0) = sx() && u
  def sr(dummy: Int = 0) = leaf() && r
  def sw(dummy: Int = 0) = leaf() && w && d
  def sx(dummy: Int = 0) = leaf() && x
}

class PCodeLockCfg(implicit p: Parameters) extends CoreBundle()(p) {
  require(paddrBits > 12)
  val base = UInt(width = paddrBits-12)
  val mask = UInt(width = 10)
  val valid = Bool()
  val enable = Bool()
  //val state = UInt(width = 2)
}

class PCodeLock(implicit p: Parameters) extends CoreModule()
  with HasCoreParameters {
    val io = new Bundle {
      val in = new PTE().asInput
      val out = new PTE().asOutput
      //val cfg = Vec(2 * numPCodeRanges, UInt(0,32.W)).asInput
      val cfg = Vec(numPCodeRanges, new PCodeLockCfg).asInput
      //val vpoffset = new vpoffsetCfg().asInput
      //val debug = Valid(Bits(width=paddrBits))
    }
    require(paddrBits == 32)
    io.out := io.in

    val enable = io.cfg.foldLeft(Bool(true))((l, r) => l && r.enable)
//    when(enable) {
//      //printf("pcl enabled\n")
//    }
    
    def ppn_matched = io.cfg.map(
      c => c.valid && ((Cat(UInt(0xff,8),Cat(c.mask, UInt(0,width=2))) & io.in.ppn(paddrBits-13,0)) === c.base)
    ).fold(Bool(false))( (l: Bool,r: Bool) => l || r)
    
    //val vp_matched = Mux(io.vpoffset.offset === 0, Bool(true), Mux((io.vpoffset.offset === (io.vpoffset.vaddr - io.in.ppn(paddrBits-13,0))), Bool(true), Bool(false))) 
        //printf("io.vpoffset.offset: %x\n", io.vpoffset.offset)
        //printf("io.vpoffset.vaddr: %x \n",io.vpoffset.vaddr)
        //printf("io.in.ppn(paddrBits-13,0) -> %x\n", io.in.ppn(paddrBits-13,0)) 
//  
    def in_pcode = enable && io.in.leaf() && ppn_matched
    def out_pcode = enable && io.in.leaf() && !ppn_matched
    def bypass = !enable || !io.in.leaf()
   
    
    io.out.x := Mux(bypass || io.in.u, io.in.x, Mux(ppn_matched /*&& vp_matched*/, Bool(true), Bool(false)))
    io.out.w := Mux(bypass, io.in.w, Mux(ppn_matched/* && vp_matched*/, Bool(false), io.in.w))
//    
//    when(io.out.w =/= io.in.w) {
//      printf("W privilege masked by pcl: %x -> %x\n",io.in.w, io.out.w)
//    }
//    when(io.out.x =/= io.in.x) {
//      printf("X privilege masked by pcl: %x -> %x\n",io.in.x, io.out.x)
//    }
    
    /*io.debug.bits := Cat(
      Bool(true), ppn_matched, io.in.w,io.in.x, debug_count, io.in.ppn(19,0))
    */
    /*
    io.debug.valid := Mux(bypass, Bool(false),
      (ppn_matched && (io.in.w === Bool(true) || io.in.x === Bool(false))) ||
      (!ppn_matched && io.in.u === Bool(false) && io.in.x === Bool(true))
      )*/
  }

@chiselName
class PTW(n: Int)(implicit edge: TLEdgeOut, p: Parameters) extends CoreModule()(p) {
  val io = new Bundle {
    val requestor = Vec(n, new TLBPTWIO).flip
    val mem = new HellaCacheIO
    val dpath = new DatapathPTWIO
  }

  /*
   * priv-code-block
   * range store and interface.
   */
  
  val pcode_cfg = Reg(init = Vec.fill(numPCodeRanges){ new PCodeLockCfg().fromBits(0)} )
  val pcode_resp = Reg(Valid(new PCodeUpdate))
  //val vpoffset_resp = Reg(Valid(new vpoffsetUpdate))
//val s_idle :: s_enabled :: s_demoted :: s_verifying :: Nil = Enum(UInt(), 4)
    val vpoffset_cfg = Reg(Valid(new vpoffsetUpdate))
 
 if (hasPrivCodeLock) {
  
    require(numPCodeRanges <= 4)
    require(paddrBits == 32)
    io.dpath.pcode_resp := pcode_resp
    
    when(io.dpath.pcode_req.valid) {
//      printf("PCodeLockCfg Updated with id: %x, base: %x, mask: %x, enable: %x, lock: %x\n",
//        io.dpath.pcode_req.bits.id,
//        io.dpath.pcode_req.bits.value.base,
//        io.dpath.pcode_req.bits.value.mask,
//        io.dpath.pcode_req.bits.value.valid,
//        io.dpath.pcode_req.bits.value.locked
//        )
//
      pcode_cfg(io.dpath.pcode_req.bits.id).base := io.dpath.pcode_req.bits.value.base
      pcode_cfg(io.dpath.pcode_req.bits.id).mask := io.dpath.pcode_req.bits.value.mask
      pcode_cfg(io.dpath.pcode_req.bits.id).valid := io.dpath.pcode_req.bits.value.valid
      pcode_cfg(io.dpath.pcode_req.bits.id).enable := io.dpath.pcode_req.bits.value.locked

      //when (io.dpath.pcode_req.bits.value.enable && io.dpath.pcode_req.bits.value.locked) {
      //  pcode_cfg(io.dpath.pcode_req.bits.id).state := s_enabled 
      //}
      pcode_resp.bits := io.dpath.pcode_req.bits
    }
  }


  val s_ready :: s_req :: s_wait1 :: s_dummy1 :: s_wait2 :: s_wait3 :: s_dummy2 :: s_fragment_superpage :: Nil = Enum(UInt(), 8)
  val state = Reg(init=s_ready)

  val arb = Module(new RRArbiter(Valid(new PTWReq), n))
  arb.io.in <> io.requestor.map(_.req)
  arb.io.out.ready := state === s_ready
   
  val resp_valid = Reg(next = Vec.fill(io.requestor.size)(Bool(false)))
  val clock_en = state =/= s_ready || arb.io.out.valid || io.dpath.sfence.valid || io.dpath.customCSRs.disableDCacheClockGate
  val gated_clock =
    if (!usingVM || !tileParams.dcache.get.clockGate) clock
    else ClockGate(clock, clock_en, "ptw_clock_gate")
  withClock (gated_clock) { // entering gated-clock domain

  val invalidated = Reg(Bool())
  val count = Reg(UInt(width = log2Up(pgLevels)))
  val resp_ae = RegNext(false.B)
  val resp_fragmented_superpage = RegNext(false.B)

  val r_req = Reg(new PTWReq)
  val r_req_dest = Reg(Bits())
  val r_pte = Reg(new PTE)

  val (pte, invalid_paddr) = {
    val tmp = new PTE().fromBits(io.mem.resp.bits.data_word_bypass)
    val res = Wire(init = new PTE().fromBits(io.mem.resp.bits.data_word_bypass))
    res.ppn := tmp.ppn(ppnBits-1, 0)
    when (tmp.r || tmp.w || tmp.x) {
      // for superpage mappings, make sure PPN LSBs are zero
      for (i <- 0 until pgLevels-1)
        when (count <= i && tmp.ppn((pgLevels-1-i)*pgLevelBits-1, (pgLevels-2-i)*pgLevelBits) =/= 0) { res.v := false }
    }
    (res, (tmp.ppn >> ppnBits) =/= 0)
  }
  val traverse = pte.table() && !invalid_paddr && count < pgLevels-1
  val pte_addr = if (!usingVM) 0.U else {
    val vpn_idxs = (0 until pgLevels).map(i => (r_req.addr >> (pgLevels-i-1)*pgLevelBits)(pgLevelBits-1,0))
    val vpn_idx = vpn_idxs(count)
    Cat(r_pte.ppn, vpn_idx) << log2Ceil(xLen/8)
  }
  val fragmented_superpage_ppn = {
    val choices = (pgLevels-1 until 0 by -1).map(i => Cat(r_pte.ppn >> (pgLevels*i), r_req.addr(pgLevels*i-1, 0)))
    choices(count)
  }

  when (arb.io.out.fire()) {
    r_req := arb.io.out.bits.bits
    r_req_dest := arb.io.chosen
  }

  val (pte_cache_hit, pte_cache_data) = {
    val size = 1 << log2Up(pgLevels * 2)
    val plru = new PseudoLRU(size)
    val invalid = RegInit(true.B)
    val reg_valid = Reg(UInt(size.W))
    val valid = Mux(invalid, 0.U, reg_valid)
    val tags = Reg(Vec(size, UInt(width = paddrBits)))
    val data = Reg(Vec(size, UInt(width = ppnBits)))

    val hits = tags.map(_ === pte_addr).asUInt & valid
    val hit = hits.orR
    when ((state === s_wait2 || state === s_wait3) && traverse && !hit && !invalidated) {
      val r = Mux(valid.andR, plru.replace, PriorityEncoder(~valid))
      invalid := false
      reg_valid := Mux(io.mem.resp.valid, valid | UIntToOH(r), valid & ~UIntToOH(r))
      tags(r) := pte_addr
      data(r) := pte.ppn
    }
    when (hit && state === s_req) { plru.access(OHToUInt(hits)) }
    when (io.dpath.sfence.valid && !io.dpath.sfence.bits.rs1) { invalid := true }

    for (i <- 0 until pgLevels-1)
      ccover(hit && state === s_req && count === i, s"PTE_CACHE_HIT_L$i", s"PTE cache hit, level $i")

    (hit && count < pgLevels-1, Mux1H(hits, data))
  }

  val l2_refill = RegNext(false.B)
  io.dpath.perf.l2miss := false
  val (l2_hit, l2_valid, l2_pte, l2_tlb_ram) = if (coreParams.nL2TLBEntries == 0) (false.B, false.B, Wire(new PTE), None) else {
    val code = new ParityCode
    require(isPow2(coreParams.nL2TLBEntries))
    val idxBits = log2Ceil(coreParams.nL2TLBEntries)
    val tagBits = vpnBits - idxBits

    class Entry extends Bundle {
      val tag = UInt(width = tagBits)
      val ppn = UInt(width = ppnBits)
      val d = Bool()
      val a = Bool()
      val u = Bool()
      val x = Bool()
      val w = Bool()
      val r = Bool()

      override def cloneType = new Entry().asInstanceOf[this.type]
    }

    val ram =  DescribedSRAM(
      name = "l2_tlb_ram",
      desc = "L2 TLB",
      size = coreParams.nL2TLBEntries,
      data = UInt(width = code.width(new Entry().getWidth))
    )

    val g = Reg(UInt(width = coreParams.nL2TLBEntries))
    val valid = RegInit(UInt(0, coreParams.nL2TLBEntries))
    val (r_tag, r_idx) = Split(r_req.addr, idxBits)
    when (l2_refill && !invalidated) {
      val entry = Wire(new Entry)
      entry := r_pte
      entry.tag := r_tag
      ram.write(r_idx, code.encode(entry.asUInt))

      val mask = UIntToOH(r_idx)
      valid := valid | mask
      g := Mux(r_pte.g, g | mask, g & ~mask)
    }
    when (io.dpath.sfence.valid) {
      valid :=
        Mux(io.dpath.sfence.bits.rs1, valid & ~UIntToOH(io.dpath.sfence.bits.addr(idxBits+pgIdxBits-1, pgIdxBits)),
        Mux(io.dpath.sfence.bits.rs2, valid & g, 0.U))
    }

    val s0_valid = !l2_refill && arb.io.out.fire()
    val s1_valid = RegNext(s0_valid && arb.io.out.bits.valid)
    val s2_valid = RegNext(s1_valid)
    val s1_rdata = ram.read(arb.io.out.bits.bits.addr(idxBits-1, 0), s0_valid)
    val s2_rdata = code.decode(RegEnable(s1_rdata, s1_valid))
    val s2_valid_bit = RegEnable(valid(r_idx), s1_valid)
    val s2_g = RegEnable(g(r_idx), s1_valid)
    when (s2_valid && s2_valid_bit && s2_rdata.error) { valid := 0.U }

    val s2_entry = s2_rdata.uncorrected.asTypeOf(new Entry)
    val s2_hit = s2_valid && s2_valid_bit && !s2_rdata.error && r_tag === s2_entry.tag
    io.dpath.perf.l2miss := s2_valid && !(s2_valid_bit && r_tag === s2_entry.tag)
    val s2_pte = Wire(new PTE)
    s2_pte := s2_entry
    s2_pte.g := s2_g
    s2_pte.v := true

    ccover(s2_hit, "L2_TLB_HIT", "L2 TLB hit")

    (s2_hit, s2_valid && s2_valid_bit, s2_pte, Some(ram))
  }

  // if SFENCE occurs during walk, don't refill PTE cache or L2 TLB until next walk
  invalidated := io.dpath.sfence.valid || (invalidated && state =/= s_ready)

  io.mem.req.valid := state === s_req || state === s_dummy1
  io.mem.req.bits.phys := Bool(true)
  io.mem.req.bits.cmd  := M_XRD
  io.mem.req.bits.typ  := log2Ceil(xLen/8)
  io.mem.req.bits.addr := pte_addr
  io.mem.s1_kill := l2_hit || state =/= s_wait1
  io.mem.s2_kill := Bool(false)

  val pageGranularityPMPs = pmpGranularity >= (1 << pgIdxBits)
  val pmaPgLevelHomogeneous = (0 until pgLevels) map { i =>
    val pgSize = BigInt(1) << (pgIdxBits + ((pgLevels - 1 - i) * pgLevelBits))
    if (pageGranularityPMPs && i == pgLevels - 1) {
      require(TLBPageLookup.homogeneous(edge.manager.managers, pgSize), s"All memory regions must be $pgSize-byte aligned")
      true.B
    } else {
      TLBPageLookup(edge.manager.managers, xLen, p(CacheBlockBytes), pgSize)(pte_addr).homogeneous
    }
  }
  val pmaHomogeneous = pmaPgLevelHomogeneous(count)
  val pmpHomogeneous = new PMPHomogeneityChecker(io.dpath.pmp).apply(pte_addr >> pgIdxBits << pgIdxBits, count)
  val homogeneous = pmaHomogeneous && pmpHomogeneous

//  printf("In PTW io.requestor(0).vpoffset.bits.value %x\n",  io.requestor(0).vpoffset.bits.value)
 // printf("In PTW io.requestor(1).vpoffset.bits.value %x\n",  io.requestor(1).vpoffset.bits.value)
  for (i <- 0 until io.requestor.size) {
    if (hasPrivCodeLock) {
      when(io.dpath.vpoffset_req.valid) {
        io.requestor(i).vpoffset.bits.value := io.dpath.vpoffset_req.bits.value
        io.requestor(i).vpoffset.valid := true
      }  
    }
    io.requestor(i).resp.valid := resp_valid(i)
    io.requestor(i).resp.bits.ae := resp_ae
    io.requestor(i).resp.bits.pte := r_pte
    io.requestor(i).resp.bits.level := count
    io.requestor(i).resp.bits.homogeneous := homogeneous || pageGranularityPMPs
    io.requestor(i).resp.bits.fragmented_superpage := resp_fragmented_superpage && pageGranularityPMPs
    io.requestor(i).ptbr := io.dpath.ptbr
    io.requestor(i).customCSRs := io.dpath.customCSRs
    io.requestor(i).status := io.dpath.status
    io.requestor(i).pmp := io.dpath.pmp
  }

  // control state machine
  val next_state = Wire(init = state)
  state := OptimizationBarrier(next_state)

  switch (state) {
    is (s_ready) {
      when (arb.io.out.fire()) {
        next_state := Mux(arb.io.out.bits.valid, s_req, s_ready)
      }
      count := UInt(0)
    }
    is (s_req) {
      when (pte_cache_hit) {
        count := count + 1
      }.otherwise {
        next_state := Mux(io.mem.req.ready, s_wait1, s_req)
      }
    }
    is (s_wait1) {
      next_state := s_wait2
    }
    is (s_wait2) {
      next_state := s_wait3
      when (io.mem.s2_xcpt.ae.ld) {
        resp_ae := true
        next_state := s_ready
        resp_valid(r_req_dest) := true
      }
    }
    is (s_fragment_superpage) {
      next_state := s_ready
      resp_valid(r_req_dest) := true
      resp_ae := false
      when (!homogeneous) {
        count := pgLevels-1
        resp_fragmented_superpage := true
      }
    }
  }

  def makePTE(ppn: UInt, default: PTE) = {
    val pte = Wire(init = default)
    pte.ppn := ppn
    pte
  }
  val pCodeLock = Module(new PCodeLock())
/* 
  val count_debug_queue = Reg(init=UInt(0, 32.W))
  val debug_queue = Module(new Queue(UInt(32.W), 64))
  io.dpath.pcode_debug.valid := debug_queue.io.deq.valid
  debug_queue.io.deq.ready := io.dpath.pcode_debug.ready || 
    count_debug_queue === UInt(60)
  debug_queue.io.enq.valid := pCodeLock.io.debug.valid
  debug_queue.io.enq.bits := pCodeLock.io.debug.bits
  io.dpath.pcode_debug.bits := Mux(debug_queue.io.deq.valid, debug_queue.io.deq.bits, UInt(0))
  
  when(debug_queue.io.deq.fire()) {
    printf("[counter_debug_queue] deq: %x\n",debug_queue.io.deq.bits)
    when(debug_queue.io.enq.fire()) {
      printf("[counter_debug_queue] enq: %x\n",debug_queue.io.enq.bits)
      count_debug_queue := count_debug_queue
    }.otherwise {
      count_debug_queue := count_debug_queue - 1
      printf("counter_debug_queue: %d -> %d\n",count_debug_queue,count_debug_queue-1)
    }
  }.otherwise {
    when(debug_queue.io.enq.fire()) {
      printf("[counter_debug_queue] enq: %x\n",debug_queue.io.enq.bits)
      count_debug_queue := count_debug_queue + 1
      printf("counter_debug_queue: %d -> %d\n",count_debug_queue,count_debug_queue+1)
    }.otherwise {
      count_debug_queue := count_debug_queue
    }

  }
*/
  def r_pte_pre = OptimizationBarrier(
    Mux(io.mem.resp.valid, pte,
    Mux(l2_hit, l2_pte,
    Mux(state === s_fragment_superpage && !homogeneous, makePTE(fragmented_superpage_ppn, r_pte),
    Mux(state === s_req && pte_cache_hit, makePTE(pte_cache_data, l2_pte),
    Mux(arb.io.out.fire(), makePTE(io.dpath.ptbr.ppn, r_pte),
    r_pte))))))

  pCodeLock.io.cfg := pcode_cfg
  pCodeLock.io.in := r_pte_pre
  r_pte := pCodeLock.io.out

  when (l2_hit) {
    assert(state === s_req || state === s_wait1)
    next_state := s_ready
    resp_valid(r_req_dest) := true
    resp_ae := false
    count := pgLevels-1
  }
  when (io.mem.s2_nack) {
    assert(state === s_wait2)
    next_state := s_req
  }
  when (io.mem.resp.valid) {
    assert(state === s_wait2 || state === s_wait3)
    when (traverse) {
      next_state := s_req
      count := count + 1
    }.otherwise {
      l2_refill := pte.v && !invalid_paddr && count === pgLevels-1
      val ae = pte.v && invalid_paddr
      resp_ae := ae
      when (pageGranularityPMPs && count =/= pgLevels-1 && !ae) {
        next_state := s_fragment_superpage
      }.otherwise {
        next_state := s_ready
        resp_valid(r_req_dest) := true
      }
    }
  }

  for (i <- 0 until pgLevels) {
    val leaf = io.mem.resp.valid && !traverse && count === i
    ccover(leaf && pte.v && !invalid_paddr, s"L$i", s"successful page-table access, level $i")
    ccover(leaf && pte.v && invalid_paddr, s"L${i}_BAD_PPN_MSB", s"PPN too large, level $i")
    ccover(leaf && !io.mem.resp.bits.data_word_bypass(0), s"L${i}_INVALID_PTE", s"page not present, level $i")
    if (i != pgLevels-1)
      ccover(leaf && !pte.v && io.mem.resp.bits.data_word_bypass(0), s"L${i}_BAD_PPN_LSB", s"PPN LSBs not zero, level $i")
  }
  ccover(io.mem.resp.valid && count === pgLevels-1 && pte.table(), s"TOO_DEEP", s"page table too deep")
  ccover(io.mem.s2_nack, "NACK", "D$ nacked page-table access")
  ccover(state === s_wait2 && io.mem.s2_xcpt.ae.ld, "AE", "access exception while walking page table")

  } // leaving gated-clock domain

  private def ccover(cond: Bool, label: String, desc: String)(implicit sourceInfo: SourceInfo) =
    if (usingVM) cover(cond, s"PTW_$label", "MemorySystem;;" + desc)
}

/** Mix-ins for constructing tiles that might have a PTW */
trait CanHavePTW extends HasTileParameters with HasHellaCache { this: BaseTile =>
  val module: CanHavePTWModule
  var nPTWPorts = 1
  nDCachePorts += usingPTW.toInt
}

trait CanHavePTWModule extends HasHellaCacheModule {
  val outer: CanHavePTW
  val ptwPorts = ListBuffer(outer.dcache.module.io.ptw)
  val ptw = Module(new PTW(outer.nPTWPorts)(outer.dcache.node.edges.out(0), outer.p))
  if (outer.usingPTW)
    dcachePorts += ptw.io.mem
}
