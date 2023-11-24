/*
 * Copyright (C) 2012 Regents of the University of California
 * Copyright (C) 2014 Darius Rad <darius@bluespec.com>
 * Copyright (C) 2017 SiFive
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/gfp.h>
#include <asm/pgtable.h>
#include <linux/dcache.h>
#include <linux/file.h>

static long riscv_sys_mmap(unsigned long addr, unsigned long len,
			   unsigned long prot, unsigned long flags,
			   unsigned long fd, off_t offset,
			   unsigned long page_shift_offset)
{
	if (unlikely(offset & (~PAGE_MASK >> page_shift_offset)))
		return -EINVAL;
	return sys_mmap_pgoff(addr, len, prot, flags, fd,
			      offset >> (PAGE_SHIFT - page_shift_offset));
}

#ifdef CONFIG_64BIT
SYSCALL_DEFINE6(mmap, unsigned long, addr, unsigned long, len,
	unsigned long, prot, unsigned long, flags,
	unsigned long, fd, off_t, offset)
{
	return riscv_sys_mmap(addr, len, prot, flags, fd, offset, 0);
}
#else
SYSCALL_DEFINE6(mmap2, unsigned long, addr, unsigned long, len,
	unsigned long, prot, unsigned long, flags,
	unsigned long, fd, off_t, offset)
{
	/*
	 * Note that the shift for mmap2 is constant (12),
	 * regardless of PAGE_SIZE
	 */
	return riscv_sys_mmap(addr, len, prot, flags, fd, offset, 12);
}
#endif /* !CONFIG_64BIT */

#ifdef CONFIG_SMP
/*
 * Allows the instruction cache to be flushed from userspace.  Despite RISC-V
 * having a direct 'fence.i' instruction available to userspace (which we
 * can't trap!), that's not actually viable when running on Linux because the
 * kernel might schedule a process on another hart.  There is no way for
 * userspace to handle this without invoking the kernel (as it doesn't know the
 * thread->hart mappings), so we've defined a RISC-V specific system call to
 * flush the instruction cache.
 *
 * sys_riscv_flush_icache() is defined to flush the instruction cache over an
 * address range, with the flush applying to either all threads or just the
 * caller.  We don't currently do anything with the address range, that's just
 * in there for forwards compatibility.
 */
SYSCALL_DEFINE3(riscv_flush_icache, uintptr_t, start, uintptr_t, end,
	uintptr_t, flags)
{
	struct mm_struct *mm = current->mm;
	bool local = (flags & SYS_RISCV_FLUSH_ICACHE_LOCAL) != 0;

	/* Check the reserved flags. */
	if (unlikely(flags & ~SYS_RISCV_FLUSH_ICACHE_ALL))
		return -EINVAL;

	flush_icache_mm(mm, local);

	return 0;
}
#endif

#define SET_PCL 10
#define PCL_TEST_LOAD_PCODE 20
#define PCL_TEST_STORE_PCODE 21
#define PCL_TEST_EXECUTE_DATA 22
#define PCL_TEST_WRITE_PCODE_REMAP 23
#define PCL_TEST_PATCH_SYS_READ 24
#define PCL_TEST_SHUFFLE 25

extern int kptr_restrict;
void make_writable_noremap(unsigned long vaddr) {
    struct mm_struct* mm;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;
    pgd_t *pgd;
    //, new_pgd;


    mm = current->mm;
    pgd = pgd_offset(mm,vaddr);
    p4d = p4d_offset(pgd,vaddr);
    pud = pud_offset(p4d,vaddr);
    pmd = pmd_offset(pud,vaddr);
    pte = pte_offset_kernel(pmd,vaddr);
    /*
    pr_info("pgd @ %lx\n",(unsigned long)pgd);
    pr_info("p4d @ %lx\n",(unsigned long)p4d);
    pr_info("pud @ %lx\n",(unsigned long)pud);
    pr_info("pmd @ %lx\n",(unsigned long)pmd);
    pr_info("pte @ %lx\n",(unsigned long)pte);
*/
/*
    pr_info("%lx writable: %d (%lx)\n",vaddr,
            pte_write(*pte), (unsigned long)pte);
  */  
    *pte = pte_mkwrite(*pte);
    /*
    pr_info("%lx writable: %d (%lx)\n",vaddr,
            pte_write(*pte), (unsigned long)pte);
    */
    flush_tlb_mm(mm);

}


void make_writable(unsigned long vaddr) {
    struct mm_struct* mm;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd, new_pmd;
    pte_t* pte;
    pgd_t *pgd, new_pgd;

    pmd_t* fake_pmd;
    pte_t* fake_pt;
    //pgd_t fake_pgd;

    kptr_restrict = 0;
    mm = current->mm;
    pgd = pgd_offset(mm,vaddr);
    p4d = p4d_offset(pgd,vaddr);
    pud = pud_offset(p4d,vaddr);
    pmd = pmd_offset(pud,vaddr);
    pte = pte_offset_kernel(pmd,vaddr);
    pr_info("pgd @ %lx\n",(unsigned long)pgd);
    pr_info("p4d @ %lx\n",(unsigned long)p4d);
    pr_info("pud @ %lx\n",(unsigned long)pud);
    pr_info("pmd @ %lx\n",(unsigned long)pmd);
    pr_info("pte @ %lx\n",(unsigned long)pte);

    make_writable_noremap(pgd_page_vaddr(*pgd));
    
    fake_pmd = (pmd_t *)__get_free_page(
		GFP_KERNEL | __GFP_RETRY_MAYFAIL | __GFP_ZERO);
    fake_pt = (pte_t *)__get_free_page(
            GFP_KERNEL | __GFP_RETRY_MAYFAIL | __GFP_ZERO);

    memcpy(fake_pmd, (void*)((unsigned long)pmd & ~(0x1000-1)), PAGE_SIZE);
    pr_info("done copying pmd @ %lx\n", (unsigned long)(fake_pmd));
    memcpy(fake_pt, (void*)((unsigned long)pte & ~(0x1000-1)), PAGE_SIZE);
    pr_info("done copying pt @ %lx\n", (unsigned long)(fake_pt));
    
    new_pgd = pfn_pgd(PFN_DOWN((uintptr_t)(__pa(fake_pmd))),
                __pgprot(_PAGE_TABLE));
    new_pmd = pfn_pmd(PFN_DOWN((uintptr_t)(__pa(fake_pt))),
            __pgprot(pgprot_val(PAGE_KERNEL) |_PAGE_EXEC));
    pr_info("old: %lx, new: %lx\n",pgd_val(*pgd), pgd_val(new_pgd));
    pr_info("old virt: %lx\n",pgd_page_vaddr(*pgd));
    pr_info("new virt: %lx\n",pgd_page_vaddr(new_pgd));

    pr_info("old: %lx, new: %lx\n",pmd_val(*pmd), pmd_val(new_pmd));
    pr_info("old virt: %lx\n",pmd_page_vaddr(*pmd));
    pr_info("new virt: %lx\n",pmd_page_vaddr(new_pmd));





    
    //fake_pgd = new_pgd;
    //mm->pgd = &fake_pgd;
    set_pgd(pgd, new_pgd);
    pr_info("old virt again: %lx\n",pgd_page_vaddr(*pgd));

    
    pgd = pgd_offset(mm,vaddr);
    p4d = p4d_offset(pgd,vaddr);
    pud = pud_offset(p4d,vaddr);
    pmd = pmd_offset(pud,vaddr);
    pte = pte_offset_kernel(pmd,vaddr);
    pr_info("pgd @ %lx\n",(unsigned long)pgd);
    pr_info("p4d @ %lx\n",(unsigned long)p4d);
    pr_info("pud @ %lx\n",(unsigned long)pud);
    pr_info("pmd @ %lx\n",(unsigned long)pmd);
    pr_info("pte @ %lx\n",(unsigned long)pte);

    pr_info("old: %lx, new: %lx\n",pmd_val(*pmd), pmd_val(new_pmd));
    pr_info("old virt: %lx\n",pmd_page_vaddr(*pmd));
    pr_info("new virt: %lx\n",pmd_page_vaddr(new_pmd));


    set_pmd(pmd, new_pmd);

    
    pgd = pgd_offset(mm,vaddr);
    p4d = p4d_offset(pgd,vaddr);
    pud = pud_offset(p4d,vaddr);
    pmd = pmd_offset(pud,vaddr);
    pte = pte_offset_kernel(pmd,vaddr);
    pr_info("pgd @ %lx\n",(unsigned long)pgd);
    pr_info("p4d @ %lx\n",(unsigned long)p4d);
    pr_info("pud @ %lx\n",(unsigned long)pud);
    pr_info("pmd @ %lx\n",(unsigned long)pmd);
    pr_info("pte @ %lx\n",(unsigned long)pte);


        
    pr_info("%lx writable: %d (%lx)\n",vaddr,
            pte_write(*pte), (unsigned long)pte);
    //*pte = pte_mkwrite(*pte);
    //pr_info("%lx writable: %d (%lx)\n",vaddr,
            //pte_write(*pte), (unsigned long)pte);
    flush_tlb_mm(mm);
    kptr_restrict = 0;

}

unsigned long doublemap(unsigned long vaddr) {
    struct page* page;
    uintptr_t dmap_base;
    unsigned long target;
    page = pfn_to_page(__pa(vaddr) >> PAGE_SHIFT);
    dmap_base = (uintptr_t)vmap(&page, 1, VM_MAP, PAGE_KERNEL_EXEC);
    pr_info("dmap_base: %lx\n",dmap_base);
    target = (dmap_base | (vaddr & (PAGE_SIZE-1)));
    pr_info("%lx remapped to %lx\n",vaddr, target);
    return target;
}



void dump_bytes(unsigned long* base, size_t size) {
    size_t i;
    for(i = 0; i < size; i += 1) {
        printk(KERN_INFO " %lx",*(base+i));
        if (size % 4 == 0) printk("\n");
    }
}

static unsigned int count_read_hook;
unsigned int read_hook(unsigned int fd, char __user* buf, size_t count) {
    struct fd f;
    f = fdget_pos(fd);
    count_read_hook += 1;
    if(f.file) {
        char buf[512];
        char* path = d_path(&(f.file->f_path),buf, 512);
        //pr_info("read_hook triggered for the first time\n");
        //pr_info("file name: %s\n",path);
        if(strncmp(path,"/root/victim.txt",strlen("/root/victim.txt")) == 0) {
            pr_info("%s: found the match\n",__func__);
            pr_info("%s: file name: %s\n",__func__,path);
			//skip
            return 0;
        }
    } 
    //temp = 1;
    return 1;
}

extern unsigned long hook_read_trampoline;
extern unsigned long hook_patch;
extern unsigned long hook_patch_end;
//extern unsigned long* patch_target;

void hijack_sys_read(void) {
    unsigned long orig_read;
    unsigned long trampoline;
    unsigned long hook_patch_loc;
    unsigned long hook_patch_size;
    unsigned long remapped;
    count_read_hook = 0;
    orig_read = (unsigned long)sys_read;
    trampoline = (unsigned long)(&hook_read_trampoline);
    hook_patch_loc = (unsigned long)(&hook_patch);
    hook_patch_size = (unsigned long)(&hook_patch_end) - hook_patch_loc;
    pr_info("orig_read: %lx, trampoline: %lx, hook_patch_loc: %lx, hook_patch_size: %lx\n",
            orig_read, trampoline, hook_patch_loc, hook_patch_size);
    remapped = doublemap(orig_read);
    memcpy((void*)remapped, (const void*)hook_patch_loc, hook_patch_size);
    //memcpy(remapped, orig_read, hook_patch_size);
    vunmap((const void*)(remapped & PAGE_MASK));
}

/*
 * Unit test for data page execution prevention
 */

asmlinkage void test_injection_target(void) {
    pr_info("test_injection\n");
}

typedef void (*target_ftn_t)(void);
typedef void (*hook_ftn_t)(target_ftn_t);

void test_injection(target_ftn_t target){
    target();
}


void test_execute_data(void) {
    unsigned long injected_page, executable_mapping, orig_code;
    hook_ftn_t injected_code;
    struct page* page;
    injected_page = __get_free_page(
            GFP_KERNEL | __GFP_RETRY_MAYFAIL | __GFP_ZERO);
    page = pfn_to_page(__pa(injected_page) >> PAGE_SHIFT);
    executable_mapping = (unsigned long)vmap(&page, 1, VM_MAP, PAGE_KERNEL_EXEC);
    orig_code = (unsigned long)test_injection & ~(0xF);
    memcpy((void*)injected_page, (const void*)orig_code, PAGE_SIZE);
    injected_code = (hook_ftn_t)(executable_mapping | ((unsigned long)test_injection & (0xF)));
    test_injection(test_injection_target);

    pr_info("using test_injection: %lx\n", (unsigned long)test_injection);
    //dump_bytes((unsigned long*)((unsigned long)test_injection & ~(0xF)), 4);
    pr_info("using injected code: %lx\n",(unsigned long)injected_code);
    //dump_bytes((unsigned long*)((unsigned long)injected_code & ~(0xF)), 4);
    injected_code(test_injection_target);
}



/*
 * Unit test for load protection.
 * By default, will pass.
 */

void test_load_pcl_vaddr(unsigned long vaddr) {
    unsigned long target;
    volatile uint64_t* pt;
    volatile uint64_t temp;
    target = vaddr;
    pt = (volatile uint64_t*)target;
    temp = *pt;
    pr_info("[%s]: loaded: %lx, %llx\n",__func__,target, temp);
}

void test_load_pcl(void) {
    unsigned long target;
    target = (unsigned long)sys_read;
    test_load_pcl_vaddr(target);
}


/*
 * Unit test for store protection under double mapping
 */

void dummy_code(void) {
    pr_err("%s\n",__func__);
}

void dummy_code_2(char* msg) {
}

void test_store_pcl_vaddr_remap(unsigned long vaddr) {
    unsigned long target;
    volatile uint8_t* pt;
    volatile uint8_t temp;
    target = doublemap(vaddr); // bypass the protection by page table
    pt = (volatile uint8_t*)target;
    temp = *pt;
    pr_info("[%s]: loaded: %lx (%x)\n",__func__,target, temp);
    mb();
    *pt = temp;
    mb();
    pr_info("[%s]: stored back to: %lx (pa: %lx), (%x)\n",__func__,target, __pa(vaddr), temp);
    vunmap((const void*)(target & PAGE_MASK));
}

void test_store_pcl_remap(void) {
    unsigned long target;
    target = (unsigned long)dummy_code;
    test_store_pcl_vaddr_remap(target);
}



/*
 * Unit test for store protection.
 */
void test_store_pcl_vaddr(unsigned long vaddr) {
    unsigned long target;
    volatile uint8_t* pt;
    volatile uint8_t temp;
    target = vaddr;
    pt = (volatile uint8_t*)target;
    temp = *pt;
    pr_info("[%s]: loaded: %lx (%x)\n",__func__,target, temp);
    mb();
    *pt = temp;
    mb();
    pr_info("[%s]: stored back to: %lx (pa: %lx) (%x)\n",__func__,target, __pa(target), temp);
}

void test_store_pcl(void) {
    unsigned long target;
    target = (unsigned long)dummy_code;
    test_store_pcl_vaddr(target);
    
}


void test_pcl_shuffle(void) {
    dummy_code_2("Before");
    void (*pt)(char*);
    unsigned long pt_ul;
    pt_ul = doublemap(dummy_code_2);
    pt = pt_ul;
    pt("After");
    vunmap((const void*)(pt_ul & PAGE_MASK));
}

extern void priv_code_lock_all(void);
extern void dump_pcl_regs_all(void);
//#ifdef CONFIG_PCL_TEST
extern unsigned long* excp_vect_table;
SYSCALL_DEFINE1(riscv_pcl_test, unsigned long, cmd) {
    pr_info("entering %s\n",__func__);
    pr_info("exception table @ %lx\n",(unsigned long)(&excp_vect_table));
    pr_info("count: %u\n",count_read_hook);
    switch(cmd) {
        case PCL_TEST_LOAD_PCODE:
            test_load_pcl();
            break;
        case PCL_TEST_STORE_PCODE:
            test_store_pcl();
            break;
        case PCL_TEST_EXECUTE_DATA:
            test_execute_data();
            break;
        case PCL_TEST_WRITE_PCODE_REMAP:
            test_store_pcl_remap();
            break;
        case PCL_TEST_PATCH_SYS_READ:
            hijack_sys_read();
            break;
        case PCL_TEST_SHUFFLE:
            test_pcl_shuffle();
            break;
        case SET_PCL:
            priv_code_lock_all();
            dump_pcl_regs_all();
            break;
        default:
            dummy_code();
            pr_err("unknown cmd: %ld\n",cmd);
            break;
    }
    return 0;
}


//#endif





