/*
 * SMP initialisation and IPI support
 * Based on arch/arm64/kernel/smp.c
 *
 * Copyright (C) 2012 ARM Ltd.
 * Copyright (C) 2015 Regents of the University of California
 * Copyright (C) 2017 SiFive
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/sched/task_stack.h>
#include <asm/irq.h>
#include <asm/mmu_context.h>
#include <asm/tlbflush.h>
#include <asm/sections.h>
#include <asm/sbi.h>
#include <linux/kallsyms.h>

void *__cpu_up_stack_pointer[NR_CPUS];
void *__cpu_up_task_pointer[NR_CPUS];

void __init smp_prepare_boot_cpu(void)
{
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
}

void __init setup_smp(void)
{
	struct device_node *dn = NULL;
	int hart, im_okay_therefore_i_am = 0;

	while ((dn = of_find_node_by_type(dn, "cpu"))) {
		hart = riscv_of_processor_hart(dn);
		if (hart >= 0) {
			set_cpu_possible(hart, true);
			set_cpu_present(hart, true);
			if (hart == smp_processor_id()) {
				BUG_ON(im_okay_therefore_i_am);
				im_okay_therefore_i_am = 1;
			}
		}
	}

	BUG_ON(!im_okay_therefore_i_am);
}

int __cpu_up(unsigned int cpu, struct task_struct *tidle)
{
	tidle->thread_info.cpu = cpu;

	/*
	 * On RISC-V systems, all harts boot on their own accord.  Our _start
	 * selects the first hart to boot the kernel and causes the remainder
	 * of the harts to spin in a loop waiting for their stack pointer to be
	 * setup by that main hart.  Writing __cpu_up_stack_pointer signals to
	 * the spinning harts that they can continue the boot process.
	 */
	smp_mb();
	__cpu_up_stack_pointer[cpu] = task_stack_page(tidle) + THREAD_SIZE;
	__cpu_up_task_pointer[cpu] = tidle;

	while (!cpu_online(cpu))
		cpu_relax();

	return 0;
}

void __init smp_cpus_done(unsigned int max_cpus)
{
}

/*
 * ref:
 * arch/riscv/kernel/vmlinux.lds.S
 * arch/arm/mm/init.c
 */
struct priv_code_region_t {
    unsigned long start;
    unsigned long end;
    unsigned long align;
};

static int num_pcode_regions = 4;
static struct priv_code_region_t priv_code_region = {
        .start = _text,
        .end =_end_text,
        .align = (1 << 21)
};

static void set_pcl_pa(uintptr_t base, uint8_t idx) {
    uintptr_t p_base = base; 
    pr_info("setting p_base: %lx\n",p_base);
    switch(idx) {
        case 0: csr_write(0x190,p_base); break;
        case 1: csr_write(0x191,p_base); break;
        case 2: csr_write(0x192,p_base); break;
        case 3: csr_write(0x193,p_base); break;
        default: __builtin_unreachable();
    }
  asm volatile ("sfence.vma" ::: "memory");
}

inline static unsigned long get_pcl_pa(uint8_t idx, uint8_t mask) {
    unsigned long ret;
        switch(idx) {
            case 0: ret = csr_read(0x190); break;
            case 1: ret = csr_read(0x191); break;
            case 2: ret = csr_read(0x192); break;
            case 3: ret = csr_read(0x193); break;
            default: __builtin_unreachable();
        }
    return ret;
}

inline static void set_isolationsw(uint8_t value) {
    pr_info("set isolationsw setting : %x\n",value);
    csr_write(0x198,value);
  asm volatile ("sfence.vma" ::: "memory");
}





void dump_pcl_regs(void* info) {
    uint8_t i;
    unsigned int core_id = smp_processor_id();
    for(i = 0; i < 4; i+=1) {
        pr_info("[%s] pcl[%x] @ %u: %lx, %lx\n",__func__,i,core_id,get_pcl_pa(i,0), get_pcl_pa(i,1));
    }
}

void dump_pcl_regs_all(void) {
    dump_pcl_regs(NULL);
    smp_call_function(dump_pcl_regs, NULL, true);
}

static void set_priv_code_lock(void* info) {
    int i = 0;
    int num_regions_used = 0;
    uint32_t mask_2mb = ~((1 << 21) - 1);
    unsigned long num_2mbs = 0;

    pr_info("va_pa_offset: %lx\n", va_pa_offset);
    csr_write(0x182, va_pa_offset);
//	csr_write(0x182, 1);
	struct priv_code_region_t r = priv_code_region;
    unsigned long start = __pa(r.start);
	unsigned long end = __pa(r.end);
	pr_info("[set_priv_code_lock] virtual pcode region : %lx --- %lx\n", r.start, r.end);
	pr_info("[set_priv_code_lock] pcode region : %lx --- %lx\n", start, end);
        
        unsigned long size = 0;
        int j = 0;
       if(end <= start) {
            pr_err("weird region, end smaller then start\n");
        }
        size = end - start;
		pr_err("pocde size %lx\n", size);
        num_2mbs = (size >> 21) + 1;
        if(start & (~mask_2mb)) {//|| (size & mask_2mb) ) {
            pr_err("pcode region is not 2MB aligned!\n");
        }
        if (num_2mbs > 4 - num_regions_used) {
            pr_err("pcode region registers not enough\n");
        }
        //pr_info("num_2mbsXX: %lu\n", num_2mbs);
	    //for(j = num_2mbs; j != 0; j -= 1) {
		for(j=0 ;j < num_2mbs; j++) {
		    /*
             * Set one 2mb region per reg.
             */
            unsigned long  base;
            unsigned int core_id;

            unsigned int lock;
            unsigned int valid;
            uint32_t pcl_val;
            uint32_t base_20, mask_10;
            uint32_t sanity_check;
            
            valid = 0x1;
			lock = 0x1;
            base = start + (j << 21);
            core_id = smp_processor_id();
            base_20 = base >> 12;
            pr_info("mask_2mb: %x\n", mask_2mb);
            mask_10 = ((mask_2mb >> 12) >> 2) & 0x3FF;
            pr_info("num_2mbs: %lu\n", num_2mbs);
            pr_info("base: %lx\n", base);
            pr_info("base in 20 bits: %x\n", base_20);
            pr_info("mask in 10 bits: %x\n", mask_10);
            pcl_val = (base_20 << 12) | (mask_10 << 2) | (valid << 1) | lock;
            pr_info("pcl_val: %x\n", pcl_val);
            sanity_check = (base_20 << 12) == (base & ((0xFF << 24) | (mask_10 << 14)));
            pr_info("sanity_check (must be 1): %x\n", sanity_check);
            
            //pr_info("calling set_pcl_pa(base: %lx,mask:  %lx, lock: %d, j: %lu) @ core %d\n",base, mask, lock, j, core_id);
			//pr_info("(base & mask) | turn on %lx", (base & mask) | lock);
			unsigned update_value = (base_20 << 12) | (mask_10) << 2 |(valid << 1) | (lock);
			pr_info("update_value %lx", update_value);
            
            set_pcl_pa(pcl_val, j);
            
            //pr_info("%lu, %lu, %d, %d\n", j, num_2mbs, j != num_2mbs, j != (int)num_2mbs);
        }
		//Lock all of pcode_regions
		for(j=num_2mbs; j< num_pcode_regions; j++){
            pr_info("locking pcl[%d] with valid=0\n", j);
		     set_pcl_pa(0x1, j);
		}
        
        num_regions_used += num_2mbs;

    

	//dump_pcl_regs_all();
    //enable_pcl();
}
void priv_code_lock_clear(void* info) {
    int i;
    pr_info("clearing pcl");
    for(i = 0; i < num_pcode_regions ; i+=1) {
        set_pcl_pa(0,i);
    }
}


void priv_code_lock_clear_all(void) {
//#ifdef CONFIG_PRIV_CODE_LOCK
    priv_code_lock_clear(NULL);
    //smp_call_function(priv_code_lock_clear,NULL,true);
//#endif
    pr_info("[%s]: cleared pcl registers\n",__func__);
}

void priv_code_lock_all(void) {
    unsigned long end_addr;
    struct priv_code_region_t r;
    priv_code_lock_clear_all();
    end_addr = _end_text;
	//pr_info("end_addr %lx\n", end_addr); 
    end_addr = (end_addr + 0x1000) & ~(0xFFF);
    end_addr = virt_to_phys(end_addr);
    r = priv_code_region;
    //pr_info("pcode region [%d]: %lx --- %lx\n",0,r.start, r.end);
    pr_info("pcode region [%d]: %lx --- %lx\n",0,__pa(r.start), __pa(r.end));
    //pr_info("end_addr: %lx\n",end_addr);
    SBI_CALL_2(SBI_LOAD_LKM, 0, end_addr);
    set_priv_code_lock(NULL);
    //smp_call_function(set_priv_code_lock, NULL, true);
}

void arch_mark_readonly(void) {


    //priv_code_lock_clear_all();
	priv_code_lock_all();
    pr_info("will use priv-code-lock to mark readonly\n");
    pr_info("testing\n");
    return;
}


/*
 * C entry point for a secondary processor.
 */
asmlinkage void __init smp_callin(void)
{
	struct mm_struct *mm = &init_mm;

	/* All kernel threads share the same mm context.  */
	atomic_inc(&mm->mm_count);
	current->active_mm = mm;
	trap_init();
	init_clockevent();
	notify_cpu_starting(smp_processor_id());
	set_cpu_online(smp_processor_id(), 1);
	local_flush_tlb_all();
	local_irq_enable();
	preempt_disable();
	cpu_startup_entry(CPUHP_AP_ONLINE_IDLE);
}
