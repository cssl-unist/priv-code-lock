// See LICENSE for license details.

#include "mtrap.h"
#include "mcall.h"
#include "htif.h"
#include "atomic.h"
#include "bits.h"
#include "vm.h"
#include "uart.h"
#include "uart16550.h"
#include "finisher.h"
#include "fdt.h"
#include "unprivileged_memory.h"
#include "disabled_hart_mask.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "hmac_sha256.h"

#include <assert.h>
#include <stdint.h>
//#include <stdlib.h>


//#define SHA256_BLOCK_SIZE 32

void __attribute__((noreturn)) bad_trap(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc)
{
  die("machine mode: unhandlable trap %d @ %p", read_csr(mcause), mepc);
}

static uintptr_t mcall_console_putchar(uint8_t ch)
{
  if (uart) {
    uart_putchar(ch);
  } else if (uart16550) {
    uart16550_putchar(ch);
  } else if (htif) {
    htif_console_putchar(ch);
  }
  return 0;
}

void putstring(const char* s)
{
  while (*s)
    mcall_console_putchar(*s++);
}

void vprintm(const char* s, va_list vl)
{
  char buf[256];
  vsnprintf(buf, sizeof buf, s, vl);
  putstring(buf);
}

void printm(const char* s, ...)
{
  va_list vl;

  va_start(vl, s);
  vprintm(s, vl);
  va_end(vl);
}

static void send_ipi(uintptr_t recipient, int event)
{
  if (((disabled_hart_mask >> recipient) & 1)) return;
  atomic_or(&OTHER_HLS(recipient)->mipi_pending, event);
  mb();
  *OTHER_HLS(recipient)->ipi = 1;
}

static uintptr_t mcall_console_getchar()
{
  if (uart) {
    return uart_getchar();
  } else if (uart16550) {
    return uart16550_getchar();
  } else if (htif) {
    return htif_console_getchar();
  } else {
    return '\0';
  }
}

static uintptr_t mcall_clear_ipi()
{
  return clear_csr(mip, MIP_SSIP) & MIP_SSIP;
}

static uintptr_t mcall_shutdown()
{
  poweroff(0);
}

static uintptr_t mcall_set_timer(uint64_t when)
{
  *HLS()->timecmp = when;
  clear_csr(mip, MIP_STIP);
  set_csr(mie, MIP_MTIP);
  return 0;
}

static void send_ipi_many(uintptr_t* pmask, int event)
{
  _Static_assert(MAX_HARTS <= 8 * sizeof(*pmask), "# harts > uintptr_t bits");
  uintptr_t mask = hart_mask;
  if (pmask)
    mask &= load_uintptr_t(pmask, read_csr(mepc));

  // send IPIs to everyone
  for (uintptr_t i = 0, m = mask; m; i++, m >>= 1)
    if (m & 1)
      send_ipi(i, event);

  if (event == IPI_SOFT)
    return;

  // wait until all events have been handled.
  // prevent deadlock by consuming incoming IPIs.
  uint32_t incoming_ipi = 0;
  for (uintptr_t i = 0, m = mask; m; i++, m >>= 1)
    if (m & 1)
      while (*OTHER_HLS(i)->ipi)
        incoming_ipi |= atomic_swap(HLS()->ipi, 0);

  // if we got an IPI, restore it; it will be taken after returning
  if (incoming_ipi) {
    *HLS()->ipi = incoming_ipi;
    mb();
  }
}

int load_lkm_state = 0;
uint8_t* lkm_base = 0;
uint8_t* lkm_base_orig = 0;

// struct relocate_args {
// 	Elf64_Shdr rel_sechdr;
//   Elf64_Shdr sym_sechdrs;
//   char *strtab;
//   char module_name[MODULE_NAME_LEN];
// };

void dump_relocate_args(struct relocate_args* args) {
  printm("size: %lx\n", sizeof(struct relocate_args));
  //printm("sechdrs: %lx\n", (unsigned long)(args->sechdrs));
  printm("strtab: %lx\n", (unsigned long)(args->strtab));
  printm("module_name: %s\n", args->module_name);

}

void dump_shdr(Elf64_Shdr* shdr) {
	printm("sh_name: %lx\n", shdr->sh_name);
	printm("sh_type: %lx\n", shdr->sh_type);
	printm("sh_flags: %lx\n", shdr->sh_flags);
	printm("sh_addr: %lx\n", shdr->sh_addr);
	printm("sh_offset: %lx\n", shdr->sh_offset);
	printm("sh_size: %lx\n", shdr->sh_size);
	printm("sh_link: %lx\n", shdr->sh_link);
	printm("sh_info: %lx\n", shdr->sh_info);
	printm("sh_addralign: %lx\n", shdr->sh_addralign);
	printm("sh_entsize: %lx\n", shdr->sh_entsize);
}
#define SHA256_BLOCK_SIZE 64

struct copy_args {
  uintptr_t to;
  uintptr_t from;
  size_t size;
};


void do_checksum(uint8_t* dest, const uint8_t* data, size_t sum_len, size_t bytes) {
  int i = 0;
  //printm("dest: %lx, data: %lx, sum_len: %lx, bytes: %lx\n",dest, data, sum_len, bytes);
  for (i = 0; i < bytes; i+= 1) {
    *(dest + (i % sum_len)) += *(data + i);
  }
}
void mac_to_string(char* out_str, uint8_t* base, size_t size, size_t str_size) {
  unsigned i;
  
  memset(out_str, 0, str_size);
  for (i = 0; i < size; i++) {
    char buf[20];
    snprintf(buf, 20, "%x", base[i]);
    //printm("%x, %x\n", base[i], i);
    out_str[i*2 + 1] = buf[7];
    out_str[i*2] = buf[6];
  }
  
}


int hmac_verify(uint8_t* str_data) {
  // size = SHA256_BLOCK_SIZE
  
  const char* data = "4b393abced1c497f8048860ba1ede46a23f1ff5209b18e9c428bddfbb690aad84b393abced1c497f8048860ba1ede46a23f1ff5209b18e9c428bddfbb690aad8";
  const char* ref = "749995ee504b72c3b9e0d31f1ad7bf6d573e6bb6a2a7cb5d47550dbb8f4ed12fc8cd0080000000003c00608100000000f2330080000000000000000000000000";
  size_t d_len = strlen(data);
  uint8_t str_key[SHA256_BLOCK_SIZE];
  do_checksum(str_key, (const uint8_t*)data, SHA256_BLOCK_SIZE, d_len);
  uint8_t out[SHA256_BLOCK_SIZE];
  char out_str[SHA256_BLOCK_SIZE * 2 + 1];
  unsigned i;

  // Call hmac-sha256 function
  hmac_sha256(str_key, SHA256_BLOCK_SIZE, str_data, SHA256_BLOCK_SIZE, &out,
              sizeof(out));
  mac_to_string(out_str, out, SHA256_BLOCK_SIZE, SHA256_BLOCK_SIZE * 2 + 1);
  




  // Print out the result
  // printm("Message: %s\n", str_data);
  // printm("Key: %s\n", str_key);
  printm("HMAC match? (must be 0): %x\n", strcmp(out_str, ref));
  printm("HMAC of loaded: %s\n", out_str);
  printm("HMAC  expected: %s\n", ref);
  

  // This assertion fails if something went wrong
  
  return 0;
}



unsigned int load_lkm(int cmd, uintptr_t arg1) {
  unsigned int size;
  int i = 0;
  struct relocate_args* args;
  struct simplify_args* s_args;
  struct copy_args* copy_args;
  //printm("lkm base: %lx\n", (unsigned long)lkm_base);
  uint8_t checksum[SHA256_BLOCK_SIZE];
  uint8_t mac[SHA256_BLOCK_SIZE];
  uint8_t* from;
  uint8_t* to;
  
  
  switch (cmd) {
    case 0: // set lkm_base
      lkm_base = (uint8_t*)arg1;
      lkm_base += 0x1000;
      lkm_base = (uint8_t*)((uintptr_t)lkm_base & ~(0xFFF));
      lkm_base_orig = lkm_base;
      printm("setting lkm base to: %lx\n", (unsigned long)arg1);
      return 0;
    case 1: // allocate
      size = *(unsigned int*)arg1;
      *(uintptr_t*)arg1 = (uintptr_t)lkm_base;
      lkm_base += size + 0x1000;
      lkm_base = (uint8_t*)((uintptr_t)lkm_base & ~(0xFFF));
      return 0;
    case 2: // copy in
    // vulnerable. should do better with allocate.
    
      copy_args = (struct copy_args*)arg1;
      //memcpy((void*)copy_args->to, (void*)copy_args->from, (copy_args->size + 0x10) & ~(0xF));
      //printm("copy called, from: %lx, to: %lx, size: %lx\n", copy_args->from, copy_args->to, copy_args->size);
      
      from = (uint8_t*)copy_args->from;
      to = (uint8_t*)copy_args->to;
      for (i = 0; i < copy_args->size; i+=1) {
        *(to+i) = *(from+i);
      }
      return 0;
    case 3: //check and relocate
       args = (struct relocate_args*)arg1;
       memset(checksum, 0, SHA256_BLOCK_SIZE);
       do_checksum(checksum, (const uint8_t*)lkm_base_orig, SHA256_BLOCK_SIZE, (unsigned long)lkm_base - (unsigned long)lkm_base_orig);
       //dump_checksum(checksum, SHA256_BLOCK_SIZE);
       hmac_verify(checksum);
       args->mod_arch.got.shdr = &(args->got_sechdr);
       args->mod_arch.plt.shdr = &(args->plt_sechdr);
       args->mod_arch.got_plt.shdr = &(args->gotplt_sechdr);
       apply_relocate_add(args);
       return 0;
    case 4: //  simplify_symbols
       s_args = (struct simplify_args*)arg1;
       // for(i = 0; i < s_args->e_shnum; i+=1) {
       //   printm("shdr[%d]: %lx (%lx, %lx)\n", i, (s_args->sechdrs + i), (uint64_t)(s_args->sechdrs + i) + /s_args->phys_offset, (uint64_t)(s_args->sechdrs + i) - s_args->phys_offset);
       // }
       pcl_simplify_symbols(s_args);
       return 0;
    default:
      printm("undefined cmd to load_lkm: %d\n", cmd);
      return -1;
  }

  //unsigned int first_word = *prot_mem_base;
  //printm("load_lkm called with %p, %lx, %lx\n", prot_mem_base, prot_mem_size, first_word);
  return -1;
}

void mcall_trap(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc)
{
  write_csr(mepc, mepc + 4);
  

  uintptr_t n = regs[17], arg0 = regs[10], arg1 = regs[11], retval, ipi_type;
  

  switch (n)
  {
    case SBI_CONSOLE_PUTCHAR:
      retval = mcall_console_putchar(arg0);
      break;
    case SBI_CONSOLE_GETCHAR:
      retval = mcall_console_getchar();
      break;
    case SBI_SEND_IPI:
      ipi_type = IPI_SOFT;
      goto send_ipi;
    case SBI_REMOTE_SFENCE_VMA:
    case SBI_REMOTE_SFENCE_VMA_ASID:
      ipi_type = IPI_SFENCE_VMA;
      goto send_ipi;
    case SBI_REMOTE_FENCE_I:
      ipi_type = IPI_FENCE_I;
send_ipi:
      send_ipi_many((uintptr_t*)arg0, ipi_type);
      retval = 0;
      break;
    case SBI_CLEAR_IPI:
      retval = mcall_clear_ipi();
      break;
    case SBI_SHUTDOWN:
      retval = mcall_shutdown();
      break;
    case SBI_SET_TIMER:
#if __riscv_xlen == 32
      retval = mcall_set_timer(arg0 + ((uint64_t)arg1 << 32));
#else
      retval = mcall_set_timer(arg0);
#endif
      break;
    case SBI_LOAD_LKM:
      retval = load_lkm((int)arg0, arg1);
      break;
    default:
      retval = -ENOSYS;
      break;
  }
  regs[10] = retval;
}

void redirect_trap(uintptr_t epc, uintptr_t mstatus, uintptr_t badaddr)
{
  write_csr(sbadaddr, badaddr);
  write_csr(sepc, epc);
  write_csr(scause, read_csr(mcause));
  write_csr(mepc, read_csr(stvec));

  uintptr_t new_mstatus = mstatus & ~(MSTATUS_SPP | MSTATUS_SPIE | MSTATUS_SIE);
  uintptr_t mpp_s = MSTATUS_MPP & (MSTATUS_MPP >> 1);
  new_mstatus |= (mstatus * (MSTATUS_SPIE / MSTATUS_SIE)) & MSTATUS_SPIE;
  new_mstatus |= (mstatus / (mpp_s / MSTATUS_SPP)) & MSTATUS_SPP;
  new_mstatus |= mpp_s;
  write_csr(mstatus, new_mstatus);

  extern void __redirect_trap();
  return __redirect_trap();
}

void pmp_trap(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc)
{
  redirect_trap(mepc, read_csr(mstatus), read_csr(mbadaddr));
}

static void machine_page_fault(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc)
{
  // MPRV=1 iff this trap occurred while emulating an instruction on behalf
  // of a lower privilege level. In that case, a2=epc and a3=mstatus.
  // a1 holds MPRV if emulating a load or store, or MPRV | MXR if loading
  // an instruction from memory.  In the latter case, we should report an
  // instruction fault instead of a load fault.
  if (read_csr(mstatus) & MSTATUS_MPRV) {
    if (regs[11] == (MSTATUS_MPRV | MSTATUS_MXR)) {
      if (mcause == CAUSE_LOAD_PAGE_FAULT)
        write_csr(mcause, CAUSE_FETCH_PAGE_FAULT);
      else if (mcause == CAUSE_LOAD_ACCESS)
        write_csr(mcause, CAUSE_FETCH_ACCESS);
      else
        goto fail;
    } else if (regs[11] != MSTATUS_MPRV) {
      goto fail;
    }

    return redirect_trap(regs[12], regs[13], read_csr(mbadaddr));
  }

fail:
  bad_trap(regs, mcause, mepc);
}

void trap_from_machine_mode(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc)
{
  uintptr_t mcause = read_csr(mcause);

  switch (mcause)
  {
    case CAUSE_LOAD_PAGE_FAULT:
    case CAUSE_STORE_PAGE_FAULT:
    case CAUSE_FETCH_ACCESS:
    case CAUSE_LOAD_ACCESS:
    case CAUSE_STORE_ACCESS:
      return machine_page_fault(regs, mcause, mepc);
    default:
      bad_trap(regs, dummy, mepc);
  }
}

void poweroff(uint16_t code)
{
  printm("Power off\r\n");
  finisher_exit(code);
  if (htif) {
    htif_poweroff();
  } else {
    send_ipi_many(0, IPI_HALT);
    while (1) { asm volatile ("wfi\n"); }
  }
}
