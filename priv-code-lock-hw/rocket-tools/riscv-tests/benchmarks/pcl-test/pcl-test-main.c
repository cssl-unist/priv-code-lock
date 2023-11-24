// See LICENSE for license details.

#include "util.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define spcodelock	0x181
#define spcodebase0	0x190
#define spcodemask0	0x191
#define spcodebase1	0x192
#define spcodemask1	0x193
#define spcodebase2	0x194
#define spcodemask2	0x195
#define spcodebase3	0x196
#define spcodemask3	0x197
  
/*
 * Borrowed some code from pmp/pmp.c
 */

#define INLINE inline __attribute__((always_inline))
#define SCRATCH RISCV_PGSIZE
uintptr_t scratch[RISCV_PGSIZE / sizeof(uintptr_t)] __attribute__((aligned(RISCV_PGSIZE)));
uintptr_t l1pt[RISCV_PGSIZE / sizeof(uintptr_t)] __attribute__((aligned(RISCV_PGSIZE)));
uintptr_t l2pt[RISCV_PGSIZE / sizeof(uintptr_t)] __attribute__((aligned(RISCV_PGSIZE)));
#if __riscv_xlen == 64
uintptr_t l3pt[RISCV_PGSIZE / sizeof(uintptr_t)] __attribute__((aligned(RISCV_PGSIZE)));
#else
#define l3pt l2pt
#endif

void s_entry(void) {
}

INLINE uintptr_t va2pa(uintptr_t va)
{
  if (va < SCRATCH || va >= SCRATCH + RISCV_PGSIZE)
    exit(3);
  return va - SCRATCH + (uintptr_t)scratch;
}

INLINE void test_one(uintptr_t addr, uintptr_t size)
{
    printf("touching: %x (%x)\n",addr, va2pa(addr));
  uintptr_t new_mstatus = (read_csr(mstatus) & ~MSTATUS_MPP) | (MSTATUS_MPP & (MSTATUS_MPP >> 1)) | MSTATUS_MPRV;
  switch (size) {
    case 1: asm volatile ("csrrw %0, mstatus, %0; sb x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 2: asm volatile ("csrrw %0, mstatus, %0; sh x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
    case 4: asm volatile ("csrrw %0, mstatus, %0; sw x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#if __riscv_xlen >= 64
    case 8: asm volatile ("csrrw %0, mstatus, %0; sd x0, (%1); csrw mstatus, %0" : "+&r" (new_mstatus) : "r" (addr)); break;
#endif
    default: __builtin_unreachable();
  }
  printf("done touching\n");
}

INLINE void test_one_s(uintptr_t addr, uintptr_t size)
{
    printf("touching assuming in s: %x\n",addr);
  switch (size) {
      case 1: asm volatile ("lb x0, (%0); sb x0, (%0)" : :"r" (addr)); break;
      case 2: asm volatile ("lh x0, (%0); sh x0, (%0)" : :"r" (addr)); break;
      case 4: asm volatile ("lw x0, (%0); sw x0, (%0)" : :"r" (addr)); break;
      case 8: asm volatile ("ld x0, (%0); sd x0, (%0)" : :"r" (addr)); break;
    default: __builtin_unreachable();
  }
  printf("done touching\n");
}



static void init_pt()
{
  l1pt[0] = ((uintptr_t)l2pt >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
  l3pt[SCRATCH / RISCV_PGSIZE] = ((uintptr_t)scratch >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_A | PTE_D | PTE_V | PTE_R | PTE_W;
#if __riscv_xlen == 64
  l2pt[0] = ((uintptr_t)l3pt >> RISCV_PGSHIFT << PTE_PPN_SHIFT) | PTE_V;
  uintptr_t vm_choice = SATP_MODE_SV39;
#else
  uintptr_t vm_choice = SATP_MODE_SV32;
#endif
  write_csr(sptbr, ((uintptr_t)l1pt >> RISCV_PGSHIFT) |
                   (vm_choice * (SATP_MODE & ~(SATP_MODE<<1))));
  write_csr(pmpaddr2, -1);
  write_csr(pmpcfg0, (PMP_NAPOT | PMP_R) << 16);

  //write_csr(0x190, 0x80000000);
  //write_csr(0x191, 0x1FFF);
}

INLINE static void set_pcl_pa(uintptr_t base, uintptr_t mask, uint8_t idx) {
    uintptr_t p_base = base;
    printf("setting p_base: %x\n",p_base);
    switch(idx) {
        case 0: write_csr(0x190,p_base >> 12); break;
        case 1: write_csr(0x192,p_base >> 12); break;
        case 2: write_csr(0x194,p_base >> 12); break;
        case 3: write_csr(0x196,p_base >> 12); break;
        default: __builtin_unreachable();
    }

    if (mask & 0xFFFFF000 == 0) {
        printf("unsafe mask: %x\n",mask);
        //exit(1);
    }
    switch(idx) {
        case 0: write_csr(0x191,mask & 0xFFF); break;
        case 1: write_csr(0x193,mask & 0xFFF); break;
        case 2: write_csr(0x195,mask & 0xFFF); break;
        case 3: write_csr(0x197,mask & 0xFFF); break;
        default: __builtin_unreachable();
    }
  asm volatile ("sfence.vma" ::: "memory");
}

INLINE static void enable_pcl(void) {
    printf("enabling pcl\n");
    write_csr(0x181,0x2);
  //asm volatile ("sfence.vma" ::: "memory");
}

INLINE static void disable_pcl(void) {
    write_csr(0x181,0x0);
    printf("disabling pcl\n");
  //asm volatile ("sfence.vma" ::: "memory");
}



INLINE static void set_pcl(uintptr_t base, uintptr_t mask, uint8_t idx) {
    uintptr_t p_base = va2pa(base);
    set_pcl_pa(p_base,mask, idx);
}


uintptr_t handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32]) {

    printf("inside a custom trap!\n");
    printf("mstatus: %x\n",read_csr(mstatus));
    printf("cause: %x\n, epc: %x",cause, epc);
    exit(0);
}

int main( int argc, char* argv[] )
{
    // seems starts at machine mode.

    printf("pcl-test\n");
    printf("(1) initialize the page table and configure spcode* registers\n");
    printf("scratch: %p\n",scratch);
    init_pt();
    printf("(2) set scratch to be pcode\n");
    set_pcl(SCRATCH, 0xFFF, 0);
    //printf("(3) set testcode to be pcode\n");
    //set_pcl(0x80000000, 0xFFE, 1);
    printf("(3) enable pcl\n");
    enable_pcl();
    printf("(4) touch scratch\n");
    //test_one((uintptr_t)scratch+8,4);
    test_one((uintptr_t)SCRATCH + 8,4);
    printf("test done.\n");

    return 0;
}
