
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

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

/* ELF register definitions */
typedef unsigned long elf_greg_t;
typedef struct user_regs_struct elf_gregset_t;
#define ELF_NGREG (sizeof(elf_gregset_t) / sizeof(elf_greg_t))

typedef union __riscv_fp_state elf_fpregset_t;

#define ELF_RISCV_R_SYM(r_info) ((r_info) >> 32)
#define ELF_RISCV_R_TYPE(r_info) ((r_info) & 0xffffffff)

/*
 * RISC-V relocation types
 */

/* Relocation types used by the dynamic linker */
#define R_RISCV_NONE		0
#define R_RISCV_32		1
#define R_RISCV_64		2
#define R_RISCV_RELATIVE	3
#define R_RISCV_COPY		4
#define R_RISCV_JUMP_SLOT	5
#define R_RISCV_TLS_DTPMOD32	6
#define R_RISCV_TLS_DTPMOD64	7
#define R_RISCV_TLS_DTPREL32	8
#define R_RISCV_TLS_DTPREL64	9
#define R_RISCV_TLS_TPREL32	10
#define R_RISCV_TLS_TPREL64	11

/* Relocation types not used by the dynamic linker */
#define R_RISCV_BRANCH		16
#define R_RISCV_JAL		17
#define R_RISCV_CALL		18
#define R_RISCV_CALL_PLT	19
#define R_RISCV_GOT_HI20	20
#define R_RISCV_TLS_GOT_HI20	21
#define R_RISCV_TLS_GD_HI20	22
#define R_RISCV_PCREL_HI20	23
#define R_RISCV_PCREL_LO12_I	24
#define R_RISCV_PCREL_LO12_S	25
#define R_RISCV_HI20		26
#define R_RISCV_LO12_I		27
#define R_RISCV_LO12_S		28
#define R_RISCV_TPREL_HI20	29
#define R_RISCV_TPREL_LO12_I	30
#define R_RISCV_TPREL_LO12_S	31
#define R_RISCV_TPREL_ADD	32
#define R_RISCV_ADD8		33
#define R_RISCV_ADD16		34
#define R_RISCV_ADD32		35
#define R_RISCV_ADD64		36
#define R_RISCV_SUB8		37
#define R_RISCV_SUB16		38
#define R_RISCV_SUB32		39
#define R_RISCV_SUB64		40
#define R_RISCV_GNU_VTINHERIT	41
#define R_RISCV_GNU_VTENTRY	42
#define R_RISCV_ALIGN		43
#define R_RISCV_RVC_BRANCH	44
#define R_RISCV_RVC_JUMP	45
#define R_RISCV_LUI		46
#define R_RISCV_GPREL_I		47
#define R_RISCV_GPREL_S		48
#define R_RISCV_TPREL_I		49
#define R_RISCV_TPREL_S		50
#define R_RISCV_RELAX		51
#define R_RISCV_SUB6		52
#define R_RISCV_SET6		53
#define R_RISCV_SET8		54
#define R_RISCV_SET16		55
#define R_RISCV_SET32		56
#define R_RISCV_32_PCREL	57

#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline u32 chunk_load(u32* ptr_in) {
  u8* ptr = (u8*) ptr_in;
  u32 ret;
  ret = 0;
  ptr += 3;
  
  ret |= *ptr;
  
  ret = ret << 8;
  ptr -= 1;
  ret |= *ptr;
  
  ret = ret << 8;
  ptr -= 1;
  ret |= *ptr;
  
  ret = ret << 8;
  ptr -= 1;
  ret |= *ptr;
  
  return ret;
}

static inline void chunk_store(u32* ptr_in, u32 data) {
  u8 byte;
  u8* ptr = (u8*)ptr_in;
  u32 orig_data = data;

  byte = data & 0xFF;
  *ptr = byte;
  data = data >> 8;
  ptr += 1;

  byte = data & 0xFF;
  *ptr = byte;
  data = data >> 8;
  ptr += 1;

  byte = data & 0xFF;
  *ptr = byte;
  data = data >> 8;
  ptr += 1;

  byte = data & 0xFF;
  *ptr = byte;

  u32 check = chunk_load(ptr_in);
  if (orig_data != check) {
    printm("error: %x != %x (%x)\n", orig_data, check);
  }
  
  return;

}

static inline void *  ERR_PTR(long error)
{
	return (void *) error;
}

static inline long  PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long  IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline long  IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)ptr);
}

#define ELF_ST_BIND(x)		((x) >> 4)
#define ELF_ST_TYPE(x)		(((unsigned int) x) & 0xf)
#define ELF32_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x)	ELF_ST_TYPE(x)
#define ELF64_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x)	ELF_ST_TYPE(x)

/* This info is needed when parsing the symbol table */
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

struct got_entry {
	u64 symbol_addr;	/* the real variable address */
};

struct plt_entry {
	/*
	 * Trampoline code to real target address. The return address
	 * should be the original (pc+4) before entring plt entry.
	 */
	u32 insn_auipc;		/* auipc t0, 0x0                       */
	u32 insn_ld;		/* ld    t1, 0x10(t0)                  */
	u32 insn_jr;		/* jr    t1                            */
};

static inline struct got_entry *get_got_entry(u64 val,
					      const struct mod_section *sec)
{
	struct got_entry *got = (struct got_entry *)sec->shdr->sh_addr;
	int i;
	for (i = 0; i < sec->num_entries; i++) {
		if (got[i].symbol_addr == val)
			return &got[i];
	}
	return NULL;
}

static inline int get_got_plt_idx(u64 val, const struct mod_section *sec)
{
	struct got_entry *got_plt = (struct got_entry *)sec->shdr->sh_addr;
	int i;
	for (i = 0; i < sec->num_entries; i++) {
		if (got_plt[i].symbol_addr == val)
			return i;
	}
	return -1;
}

static inline struct got_entry emit_got_entry(u64 val)
{
	return (struct got_entry) {val};
}


static inline struct plt_entry *get_plt_entry(u64 val,
				      const struct mod_section *sec_plt,
				      const struct mod_section *sec_got_plt)
{
	struct plt_entry *plt = (struct plt_entry *)sec_plt->shdr->sh_addr;
	int got_plt_idx = get_got_plt_idx(val, sec_got_plt);
	if (got_plt_idx >= 0)
		return plt + got_plt_idx;
	else
		return NULL;
}

u64  module_emit_got_entry(struct mod_arch_specific* mod_arch, u64 val)
{
	struct mod_section *got_sec = &mod_arch->got;
	int i = got_sec->num_entries;
	struct got_entry *got = get_got_entry(val, got_sec);

	if (got)
		return (u64)got;

	/* There is no duplicate entry, create a new one */
	got = (struct got_entry *)got_sec->shdr->sh_addr;
	got[i] = emit_got_entry(val);

	got_sec->num_entries++;
	//BUG_ON(got_sec->num_entries > got_sec->max_entries);

	return (u64)&got[i];
}

#define OPC_AUIPC  0x0017
#define OPC_LD     0x3003
#define OPC_JALR   0x0067
#define REG_T0     0x5
#define REG_T1     0x6

static inline struct plt_entry emit_plt_entry(u64 val, u64 plt, u64 got_plt)
{
	/*
	 * U-Type encoding:
	 * +------------+----------+----------+
	 * | imm[31:12] | rd[11:7] | opc[6:0] |
	 * +------------+----------+----------+
	 *
	 * I-Type encoding:
	 * +------------+------------+--------+----------+----------+
	 * | imm[31:20] | rs1[19:15] | funct3 | rd[11:7] | opc[6:0] |
	 * +------------+------------+--------+----------+----------+
	 *
	 */
	u64 offset = got_plt - plt;
	u32 hi20 = (offset + 0x800) & 0xfffff000;
	u32 lo12 = (offset - hi20);
	return (struct plt_entry) {
		OPC_AUIPC | (REG_T0 << 7) | hi20,
		OPC_LD | (lo12 << 20) | (REG_T0 << 15) | (REG_T1 << 7),
		OPC_JALR | (REG_T1 << 15)
	};
}

u64 module_emit_plt_entry(struct mod_arch_specific* mod_arch, u64 val)
{
	struct mod_section *got_plt_sec = &mod_arch->got_plt;
	struct got_entry *got_plt;
	struct mod_section *plt_sec = &mod_arch->plt;
	struct plt_entry *plt = get_plt_entry(val, plt_sec, got_plt_sec);
	int i = plt_sec->num_entries;

	if (plt)
		return (u64)plt;

	/* There is no duplicate entry, create a new one */
	got_plt = (struct got_entry *)got_plt_sec->shdr->sh_addr;
	got_plt[i] = emit_got_entry(val);
	plt = (struct plt_entry *)plt_sec->shdr->sh_addr;
	plt[i] = emit_plt_entry(val, (u64)&plt[i], (u64)&got_plt[i]);

	plt_sec->num_entries++;
	got_plt_sec->num_entries++;
	//BUG_ON(plt_sec->num_entries > plt_sec->max_entries);

	return (u64)&plt[i];
}

static int apply_r_riscv_32_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys, Elf_Addr v)
{
	if (v != (u32)v) {
		printm("%s: value %016llx out of range for 32-bit field\n",
		       module_name, v);
		return -EINVAL;
	}
	*location_phys = v;
	return 0;
}

static int apply_r_riscv_64_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys, Elf_Addr v)
{
	*(u64 *)location_phys = v;
	///printm("location %llx, v %llx\n", *location, v);
	return 0;
}

static int apply_r_riscv_branch_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				     Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	u32 imm12 = (offset & 0x1000) << (31 - 12);
	u32 imm11 = (offset & 0x800) >> (11 - 7);
	u32 imm10_5 = (offset & 0x7e0) << (30 - 10);
	u32 imm4_1 = (offset & 0x1e) << (11 - 4);

	*location_phys = (*location_phys & 0x1fff07f) | imm12 | imm11 | imm10_5 | imm4_1;
	return 0;
}

static int apply_r_riscv_jal_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				  Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	u32 imm20 = (offset & 0x100000) << (31 - 20);
	u32 imm19_12 = (offset & 0xff000);
	u32 imm11 = (offset & 0x800) << (20 - 11);
	u32 imm10_1 = (offset & 0x7fe) << (30 - 10);

	*location_phys = (*location_phys & 0xfff) | imm20 | imm19_12 | imm11 | imm10_1;
	return 0;
}

static int apply_r_riscv_rcv_branch_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
					 Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	u16 imm8 = (offset & 0x100) << (12 - 8);
	u16 imm7_6 = (offset & 0xc0) >> (6 - 5);
	u16 imm5 = (offset & 0x20) >> (5 - 2);
	u16 imm4_3 = (offset & 0x18) << (12 - 5);
	u16 imm2_1 = (offset & 0x6) << (12 - 10);

	*(u16 *)location_phys = (*(u16 *)location_phys & 0xe383) |
		    imm8 | imm7_6 | imm5 | imm4_3 | imm2_1;
	return 0;
}

static int apply_r_riscv_rvc_jump_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				       Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	u16 imm11 = (offset & 0x800) << (12 - 11);
	u16 imm10 = (offset & 0x400) >> (10 - 8);
	u16 imm9_8 = (offset & 0x300) << (12 - 11);
	u16 imm7 = (offset & 0x80) >> (7 - 6);
	u16 imm6 = (offset & 0x40) << (12 - 11);
	u16 imm5 = (offset & 0x20) >> (5 - 2);
	u16 imm4 = (offset & 0x10) << (12 - 5);
	u16 imm3_1 = (offset & 0xe) << (12 - 10);

	*(u16 *)location_phys = (*(u16 *)location_phys & 0xe003) |
		    imm11 | imm10 | imm9_8 | imm7 | imm6 | imm5 | imm4 | imm3_1;
	return 0;
}

static int apply_r_riscv_pcrel_hi20_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
					 Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	s32 hi20;
	
	if (offset != (s32)offset) {
		printm(
		  "%s: target %016llx can not be addressed by the 32-bit offset from PC = %p\n",
		  module_name, v, location);
		return -EINVAL;
	}

	hi20 = (offset + 0x800) & 0xfffff000;
	chunk_store(location_phys, (chunk_load(location_phys) & 0xfff) | hi20);
	return 0;
}

static int apply_r_riscv_pcrel_lo12_i_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
					   Elf_Addr v)
{
	/*
	 * v is the lo12 value to fill. It is calculated before calling this
	 * handler.
	 */
  
	chunk_store(location_phys, (chunk_load(location_phys) & 0xfffff) | ((v & 0xfff) << 20));
	return 0;
}

static int apply_r_riscv_pcrel_lo12_s_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
					   Elf_Addr v)
{
	/*
	 * v is the lo12 value to fill. It is calculated before calling this
	 * handler.
	 */
	u32 imm11_5 = (v & 0xfe0) << (31 - 11);
	u32 imm4_0 = (v & 0x1f) << (11 - 4);

	*location_phys = (*location_phys & 0x1fff07f) | imm11_5 | imm4_0;
	return 0;
}

static int apply_r_riscv_hi20_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				   Elf_Addr v)
{
	s32 hi20;

	// if (IS_ENABLED(CMODEL_MEDLOW)) {
	// 	printm(
	// 	  "%s: target %016llx can not be addressed by the 32-bit offset from PC = %p\n",
	// 	  module_name, v, location);
	// 	return -EINVAL;
	// }

	hi20 = ((s32)v + 0x800) & 0xfffff000;
	*location_phys = (*location_phys & 0xfff) | hi20;
	return 0;
}

static int apply_r_riscv_lo12_i_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				     Elf_Addr v)
{
	/* Skip medlow checking because of filtering by HI20 already */
	s32 hi20 = ((s32)v + 0x800) & 0xfffff000;
	s32 lo12 = ((s32)v - hi20);
	*location_phys = (*location_phys & 0xfffff) | ((lo12 & 0xfff) << 20);
	return 0;
}

static int apply_r_riscv_lo12_s_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				     Elf_Addr v)
{
	/* Skip medlow checking because of filtering by HI20 already */
	s32 hi20 = ((s32)v + 0x800) & 0xfffff000;
	s32 lo12 = ((s32)v - hi20);
	u32 imm11_5 = (lo12 & 0xfe0) << (31 - 11);
	u32 imm4_0 = (lo12 & 0x1f) << (11 - 4);
	*location_phys = (*location_phys & 0x1fff07f) | imm11_5 | imm4_0;
	return 0;
}

static int apply_r_riscv_got_hi20_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				       Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	s32 hi20;

	/* Always emit the got entry */
	//if (IS_ENABLED(CONFIG_MODULE_SECTIONS)) {
    // assume enabled
		offset = module_emit_got_entry(mod_arch, v);
		offset = (void *)offset - (void *)location;
	// } else {
	// 	printm(
	// 	  "%s: can not generate the GOT entry for symbol = %016llx from PC = %p\n",
	// 	  module_name, v, location);
	// 	return -EINVAL;
	// }

	hi20 = (offset + 0x800) & 0xfffff000;
	*location_phys = (*location_phys & 0xfff) | hi20;
	return 0;
}

static int apply_r_riscv_call_plt_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				       Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	s32 fill_v = offset;
	u32 hi20, lo12;

	if (offset != fill_v) {
		/* Only emit the plt entry if offset over 32-bit range */
		//if (IS_ENABLED(CONFIG_MODULE_SECTIONS)) {
      //assume enabled
			offset = module_emit_plt_entry(mod_arch, v);
			offset = (void *)offset - (void *)location;
		// } else {
		// 	printm(
		// 	  "%s: target %016llx can not be addressed by the 32-bit offset from PC = %p\n",
		// 	  module_name, v, location);
		// 	return -EINVAL;
		// }
	}

	hi20 = (offset + 0x800) & 0xfffff000;
	lo12 = (offset - hi20) & 0xfff;
	chunk_store(location_phys, (chunk_load(location_phys) & 0xfff) | hi20);
	chunk_store(location_phys + 1, (chunk_load(location_phys + 1) & 0xfffff) | (lo12 << 20));
	return 0;
}

static int apply_r_riscv_call_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				   Elf_Addr v)
{
	s64 offset = (void *)v - (void *)location;
	s32 fill_v = offset;
	u32 hi20, lo12;
	//printm("DEBUG: call_rela v%lx\n", v);
	//printm("DEBUG: call_rela location %lx\n", location);	
	if (offset != fill_v) {
		printm(
		  "%s: target %016llx can not be addressed by the 32-bit offset from PC = %p\n",
		  module_name, v, location);
		return -EINVAL;
	}

	hi20 = (offset + 0x800) & 0xfffff000;
	lo12 = (offset - hi20) & 0xfff;
	*location_phys = (*location_phys & 0xfff) | hi20;
	*(location_phys + 1) = (*(location_phys + 1) & 0xfffff) | (lo12 << 20);
	return 0;
}

static int apply_r_riscv_relax_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				    Elf_Addr v)
{
	return 0;
}

static int apply_r_riscv_align_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				    Elf_Addr v)
{
	printm(
	  "%s: The unexpected relocation type 'R_RISCV_ALIGN' from PC = %p\n",
	  module_name, location);
	return -EINVAL;
}

static int apply_r_riscv_add32_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				    Elf_Addr v)
{
	*(u32 *)location_phys += (*(u32 *)v);
	return 0;
}

static int apply_r_riscv_sub32_rela(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				    Elf_Addr v)
{
	*(u32 *)location_phys -= (*(u32 *)v);
	return 0;
}


static int (*reloc_handlers_rela[]) (char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys,
				Elf_Addr v) = {
	[R_RISCV_32]			= apply_r_riscv_32_rela,
	[R_RISCV_64]			= apply_r_riscv_64_rela,
	[R_RISCV_BRANCH]		= apply_r_riscv_branch_rela,
	[R_RISCV_JAL]			= apply_r_riscv_jal_rela,
	[R_RISCV_RVC_BRANCH]		= apply_r_riscv_rcv_branch_rela,
	[R_RISCV_RVC_JUMP]		= apply_r_riscv_rvc_jump_rela,
	[R_RISCV_PCREL_HI20]		= apply_r_riscv_pcrel_hi20_rela,
	[R_RISCV_PCREL_LO12_I]		= apply_r_riscv_pcrel_lo12_i_rela,
	[R_RISCV_PCREL_LO12_S]		= apply_r_riscv_pcrel_lo12_s_rela,
	[R_RISCV_HI20]			= apply_r_riscv_hi20_rela,
	[R_RISCV_LO12_I]		= apply_r_riscv_lo12_i_rela,
	[R_RISCV_LO12_S]		= apply_r_riscv_lo12_s_rela,
	[R_RISCV_GOT_HI20]		= apply_r_riscv_got_hi20_rela,
	[R_RISCV_CALL_PLT]		= apply_r_riscv_call_plt_rela,
	[R_RISCV_CALL]			= apply_r_riscv_call_rela,
	[R_RISCV_RELAX]			= apply_r_riscv_relax_rela,
	[R_RISCV_ALIGN]			= apply_r_riscv_align_rela,
	[R_RISCV_ADD32]			= apply_r_riscv_add32_rela,
	[R_RISCV_SUB32]			= apply_r_riscv_sub32_rela,
};

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

// using only module_name
int apply_relocate_add(struct relocate_args* args)
{
  
  Elf64_Shdr *rel_sechdr = &(args->rel_sechdr);
  Elf64_Shdr *sym_sechdr = &(args->sym_sechdr);
  Elf64_Shdr *tgt_sechdr = &(args->target_sechdr);
  u64 phys_offset = args->phys_offset;
  char *strtab = args->strtab;
  char* module_name = args->module_name;

	Elf_Rela *rel = (void *) rel_sechdr->sh_addr;
  //printm("sizeof(void*): %lx\n", sizeof(void*));
	int (*handler)(char* module_name, struct mod_arch_specific* mod_arch, u32 *location, u32 *location_phys, Elf_Addr v);
	Elf_Sym *sym;
	u32 *location;
	u32 r_offset;
	unsigned int i, type;
	Elf_Addr v;
	int res;
	//printm("(from bbl) [%s:%d] %x\n", __func__, __LINE__, rel_sechdr->sh_info);

	for (i = 0; i < rel_sechdr->sh_size / sizeof(*rel); i++) {
		/* This is where to make the change */
		location = ((void *)tgt_sechdr->sh_addr) + phys_offset;
		r_offset= rel[i].r_offset;
		//printm("location %lx: \n", *location);
		//same: printm("r_offset %lx: \n", r_offset);	
		location = (void *)location + r_offset;
	
		/* This is the symbol it is referring to */
		sym = (Elf_Sym *)sym_sechdr->sh_addr
			+ ELF_RISCV_R_SYM(rel[i].r_info);
		
		//printm("(Elf_Sym *)sym_sechdr->sh_addr %llx", (Elf_Sym *)sym_sechdr->sh_addr);
		//printm("ELF_RISCV_R_SYM(rel[i].r_info) %lx\n", ELF_RISCV_R_SYM(rel[i].r_info));

	if (IS_ERR_VALUE(sym->st_value)) {
			/* Ignore unresolved weak symbol */
			if (ELF_ST_BIND(sym->st_info) == STB_WEAK)
				continue;
			printm("%s: Unknown symbol %s\n",
				   module_name, strtab + sym->st_name);
			return -ENOENT;
		}

		type = ELF_RISCV_R_TYPE(rel[i].r_info);

		if (type < ARRAY_SIZE(reloc_handlers_rela))
			handler = reloc_handlers_rela[type];
		else
			handler = NULL;
    //printm("relocation type: %x\n",type);

		if (!handler) {
			printm("%s: Unknown relocation type %u\n",
			       module_name, type);
			return -EINVAL;
		}
		//printm("sym->st_value %llx\n", sym->st_value);
		//same : printm("r_addend %llx\n", rel[i].r_addend); 
		v = sym->st_value + rel[i].r_addend;

		if (type == R_RISCV_PCREL_LO12_I || type == R_RISCV_PCREL_LO12_S) {
			unsigned int j;

			for (j = 0; j < rel_sechdr->sh_size / sizeof(*rel); j++) {
				u64 hi20_loc = tgt_sechdr->sh_addr + rel[j].r_offset + phys_offset;
				
				
				u32 hi20_type = ELF_RISCV_R_TYPE(rel[j].r_info);

				/* Find the corresponding HI20 relocation entry */
        //printm("hi20_loc: %lx (%lx), st_value: %lx", hi20_loc, hi20_loc - phys_offset, sym->st_value);
				if (hi20_loc == sym->st_value
				    && (hi20_type == R_RISCV_PCREL_HI20
					|| hi20_type == R_RISCV_GOT_HI20)) {
					s32 hi20, lo12;
					Elf_Sym *hi20_sym =
						(Elf_Sym *)sym_sechdr->sh_addr
						+ ELF_RISCV_R_SYM(rel[j].r_info);
	//	printm("hi20_sym ---> %llx\n", (Elf_Sym *)sym_sechdr->sh_addr);


				u64 hi20_sym_val =
						hi20_sym->st_value
						+ rel[j].r_addend;
		//printm("(Elf_Sym *)sym_sechdr->sh_addr %llx\n", 	(Elf_Sym *)sym_sechdr->sh_addr);
		//			printm("ELF_RISCV_R_SYM(rel[j].r_info) %llx\n", ELF_RISCV_R_SYM(rel[j].r_info));	
				
					/* Calculate lo12 */
					u64 offset = hi20_sym_val - hi20_loc;
					// if (IS_ENABLED(CONFIG_MODULE_SECTIONS)
					//     && hi20_type == R_RISCV_GOT_HI20) {
					// 	offset = module_emit_got_entry(me, hi20_sym_val);
					// 	offset = offset - hi20_loc;
					// }

					hi20 = (offset + 0x800) & 0xfffff000;
					//printm("hi20 %llx\n", hi20);
					lo12 = offset - hi20;
					//printm("lo12 %llx\n", lo12);
					v = lo12;

					break;
				}
			}
			if (j == rel_sechdr->sh_size / sizeof(*rel)) {
				printm(
				  "%s: Can not find HI20 relocation information\n",
				  module_name);
				return -EINVAL;
			}
		}
    u32* location_phys;
    location_phys = (u32*)((unsigned long)location - phys_offset);
		//printm("calling handler: %lx, location: %lx, location_phys: %lx, phys_offset: %lx\n",(unsigned long)handler, location, location_phys, phys_offset);
    
		res = handler(module_name, &(args->mod_arch), location, location_phys, v);

		//printm("Relocation: location (phys: %lx, virt: %lx)\n",  location_phys, location);
		if (res)
			return res;
	}

	return 0;
}

unsigned long virt_to_phys(unsigned long virt, unsigned long offset) {
	unsigned long ret = (virt - offset) & 0xFFFFFFFF;
	return ret;
}

/* special section indexes */
#define SHN_UNDEF	0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_LIVEPATCH	0xff20
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_HIRESERVE	0xffff

int pcl_simplify_symbols(struct simplify_args* args) {
	Elf64_Shdr *sechdrs;
	Elf64_Shdr *symsec;
	Elf_Sym *sym;
	unsigned long secbase;
	unsigned int i;
	int ret = 0;
	const struct kernel_symbol *ksym;
	
	sechdrs =  (Elf64_Shdr*)((unsigned long)args->sechdrs - args->phys_offset);
	printm("index_sym: %lx\n", args->index_sym);
	symsec = (Elf64_Shdr *)((unsigned long)(&args->sechdrs[args->index_sym]) - args->phys_offset);
	printm("symsec virt: %lx\n", &args->sechdrs[args->index_sym]);
	printm("symsec: %lx\n", symsec);
	sym = (Elf_Sym*)((unsigned long)symsec->sh_addr - args->phys_offset);

	for (i = 0; i < args->smap.num_pairs; i+=1) {
		Elf64_Addr* dest = (Elf64_Addr*)virt_to_phys((unsigned long)args->smap.pairs[i].to, args->phys_offset);
		
		printm("resolving undefs with kernel-provided map %d, %lx <- %lx\n", i, dest, args->smap.pairs[i].val);
		*dest = args->smap.pairs[i].val;
	}
	printm("strtab: %lx\n", args->strtab);
	for (i = 1; i < symsec->sh_size / sizeof(Elf_Sym); i++) {
		const char *name = args->strtab + sym[i].st_name;
		//printm("symbol name[%d]: %s, st_shndx: %lx\n", i, name, sym[i].st_shndx);
		switch (sym[i].st_shndx) {
			case SHN_COMMON:
			case SHN_LIVEPATCH:
				printm("Unexpected type: %lx\n", sym[i].st_shndx);
				break;
			case SHN_ABS:
			case SHN_UNDEF:
				printm("Nothing to do for %lx\n", sym[i].st_shndx);
				break;
			default:
			  /* Divert to percpu allocation if a percpu var. */
        if (sym[i].st_shndx == args->index_pcpu) {
          printm("Unexpected type: %lx\n", sym[i].st_shndx);
					break;
				}
        else {
          secbase = sechdrs[sym[i].st_shndx].sh_addr;
					//printm("handling %lx, %lx <-- %lx\n", sym[i].st_shndx, &sym[i].st_value, sym[i].st_value + secbase);
        	sym[i].st_value += secbase;
					break;
				}
		}



		}

	printm("returnning from %s\n",__func__);
	return 0;
}


/* Change all symbols so that st_value encodes the pointer directly. */
// int simplify_symbols(Elf64_Shdr sym_sechdr, struct module *mod, const struct load_info *info)
// {
// 	Elf_Shdr *symsec = &sym_sechdr;
// 	Elf_Sym *sym = (void *)symsec->sh_addr;
// 	unsigned long secbase;
// 	unsigned int i;
// 	int ret = 0;
// 	const struct kernel_symbol *ksym;
// 	struct copy_args args;
// 	Elf32_Addr* temp;

// 	for (i = 1; i < symsec->sh_size / sizeof(Elf_Sym); i++) {
// 		const char *name = info->strtab + sym[i].st_name;

// 		switch (sym[i].st_shndx) {
// 		case SHN_COMMON:
// 			/* Ignore common symbols */
// 			if (!strncmp(name, "__gnu_lto", 9))
// 				break;

// 			/* We compiled with -fno-common.  These are not
// 			   supposed to happen.  */
// 			pr_debug("Common symbol: %s\n", name);
// 			pr_warn("%s: please compile with -fno-common\n",
// 			       mod->name);
// 			ret = -ENOEXEC;
// 			break;

// 		case SHN_ABS:
// 			/* Don't need to do anything */
// 			//printk("Absolute symbol: 0x%08lx\n",
// 			      // (long)sym[i].st_value);
// 			break;

// 		case SHN_LIVEPATCH:
// 			/* Livepatch symbols are resolved by livepatch */
// 			break;

// 		case SHN_UNDEF:
// 			ksym = resolve_symbol_wait(mod, info, name);
// 			/* Ok if resolved.  */
// 			if (ksym && !IS_ERR(ksym)) {
// 				sym[i].st_value = ksym->value;
// 				break;
// 			}

// 			/* Ok if weak.  */
// 			if (!ksym && ELF_ST_BIND(sym[i].st_info) == STB_WEAK)
// 				break;

// 			pr_warn("%s: Unknown symbol %s (err %li)\n",
// 				mod->name, name, PTR_ERR(ksym));
// 			ret = PTR_ERR(ksym) ?: -ENOENT;
// 			break;

// 		default:
// 			/* Divert to percpu allocation if a percpu var. */
// 			if (sym[i].st_shndx == info->index.pcpu)
// 				secbase = (unsigned long)mod_percpu(mod);
// 			else
// 				secbase = info->sechdrs[sym[i].st_shndx].sh_addr;
// 			//sym[i].st_value += secbase;
// 			break;
// 		}
// 	}

// 	return ret;
// }
