// See LICENSE for license details.

#ifndef _RISCV_MTRAP_H
#define _RISCV_MTRAP_H


#include "encoding.h"

#ifdef __riscv_atomic
# define MAX_HARTS 8 // arbitrary
#else
# define MAX_HARTS 1
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "elf.h"

#define read_const_csr(reg) ({ unsigned long __tmp; \
  asm ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

static inline int supports_extension(char ext)
{
  return read_const_csr(misa) & (1 << (ext - 'A'));
}

static inline int xlen()
{
  return read_const_csr(misa) < 0 ? 64 : 32;
}

extern uintptr_t mem_size;
extern volatile uint64_t* mtime;
extern volatile uint32_t* plic_priorities;
extern size_t plic_ndevs;

typedef struct {
  volatile uint32_t* ipi;
  volatile int mipi_pending;

  volatile uint64_t* timecmp;

  volatile uint32_t* plic_m_thresh;
  volatile uintptr_t* plic_m_ie;
  volatile uint32_t* plic_s_thresh;
  volatile uintptr_t* plic_s_ie;
} hls_t;

#define MACHINE_STACK_TOP() ({ \
  register uintptr_t sp asm ("sp"); \
  (void*)((sp + RISCV_PGSIZE) & -RISCV_PGSIZE); })

// hart-local storage, at top of stack
#define HLS() ((hls_t*)(MACHINE_STACK_TOP() - HLS_SIZE))
#define OTHER_HLS(id) ((hls_t*)((void*)HLS() + RISCV_PGSIZE * ((id) - read_const_csr(mhartid))))

hls_t* hls_init(uintptr_t hart_id);
void parse_config_string();
void poweroff(uint16_t code) __attribute((noreturn));
void printm(const char* s, ...);
void vprintm(const char *s, va_list args);
void putstring(const char* s);
#define assert(x) ({ if (!(x)) die("assertion failed: %s", #x); })
#define die(str, ...) ({ printm("%s:%d: " str "\n", __FILE__, __LINE__, ##__VA_ARGS__); poweroff(-1); })

void setup_pmp();
void enter_supervisor_mode(void (*fn)(uintptr_t), uintptr_t arg0, uintptr_t arg1)
  __attribute__((noreturn));
void enter_machine_mode(void (*fn)(uintptr_t, uintptr_t), uintptr_t arg0, uintptr_t arg1)
  __attribute__((noreturn));
void boot_loader(uintptr_t dtb);
void boot_other_hart(uintptr_t dtb);

static inline void wfi()
{
  asm volatile ("wfi" ::: "memory");
}


typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif
/* 64-bit ELF base types. */
typedef __u64	Elf64_Addr;
typedef __u16	Elf64_Half;
typedef __s16	Elf64_SHalf;
typedef __u64	Elf64_Off;
typedef __s32	Elf64_Sword;
typedef __u32	Elf64_Word;
typedef __u64	Elf64_Xword;
typedef __s64	Elf64_Sxword;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef signed char		s8;
typedef short			s16;
typedef int			s32;
typedef long long		s64;

// typedef struct elf64_shdr {
//   Elf64_Word sh_name;		/* Section name, index in string tbl */
//   Elf64_Word sh_type;		/* Type of section */
//   Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
//   Elf64_Addr sh_addr;		/* Section virtual addr at execution */
//   Elf64_Off sh_offset;		/* Section file offset */
//   Elf64_Xword sh_size;		/* Size of section in bytes */
//   Elf64_Word sh_link;		/* Index of another section */
//   Elf64_Word sh_info;		/* Additional section information */
//   Elf64_Xword sh_addralign;	/* Section alignment */
//   Elf64_Xword sh_entsize;	/* Entry size if section holds table */
// } Elf64_Shdr;

typedef struct elf64_rela {
  Elf64_Addr r_offset;	/* Location at which to apply the action */
  Elf64_Xword r_info;	/* index and type of relocation */
  Elf64_Sxword r_addend;	/* Constant addend used to compute value */
} Elf64_Rela;

// typedef struct elf64_sym {
//   Elf64_Word st_name;		/* Symbol name, index in string tbl */
//   unsigned char	st_info;	/* Type and binding attributes */
//   unsigned char	st_other;	/* No defined meaning, 0 */
//   Elf64_Half st_shndx;		/* Associated section index */
//   Elf64_Addr st_value;		/* Value of the symbol */
//   Elf64_Xword st_size;		/* Associated symbol size */
// } Elf64_Sym;

#define Elf_Shdr	Elf64_Shdr
#define Elf_Sym		Elf64_Sym
#define Elf_Ehdr	Elf64_Ehdr
#define Elf_Addr	Elf64_Addr
#define Elf_Rel		Elf64_Rel
#define Elf_Rela	Elf64_Rela

#define MODULE_NAME_LEN 56

struct mod_section {
	Elf64_Shdr *shdr;
	int num_entries;
	int max_entries;
};

struct mod_arch_specific {
	struct mod_section got;
	struct mod_section plt;
	struct mod_section got_plt;
};

struct relocate_args {
	Elf64_Shdr rel_sechdr;
  Elf64_Shdr sym_sechdr;
  Elf64_Shdr target_sechdr;
  Elf64_Shdr got_sechdr;
	Elf64_Shdr plt_sechdr;
	Elf64_Shdr gotplt_sechdr;
  struct mod_arch_specific mod_arch;
  u64 phys_offset;
  char *strtab;
  char module_name[MODULE_NAME_LEN];
};


struct pcl_symbol_pair {
	uintptr_t   to;
	Elf64_Addr  val;
};

struct pcl_symbol_map {
	struct pcl_symbol_pair pairs[20];
	size_t num_pairs;
};

struct simplify_args {
  Elf64_Shdr* sechdrs;
	Elf64_Half e_shnum;
  char *strtab;
	u64 phys_offset;
  unsigned int index_sym;
  unsigned int index_pcpu;
	struct pcl_symbol_map smap;
};


extern int apply_relocate_add(struct relocate_args* args);
extern int pcl_simplify_symbols(struct simplify_args* args);

#endif // !__ASSEMBLER__

#define IPI_SOFT       0x1
#define IPI_FENCE_I    0x2
#define IPI_SFENCE_VMA 0x4
#define IPI_HALT       0x8

#define MACHINE_STACK_SIZE RISCV_PGSIZE
#define MENTRY_HLS_OFFSET (INTEGER_CONTEXT_SIZE + SOFT_FLOAT_CONTEXT_SIZE)
#define MENTRY_FRAME_SIZE (MENTRY_HLS_OFFSET + HLS_SIZE)
#define MENTRY_IPI_OFFSET (MENTRY_HLS_OFFSET)
#define MENTRY_IPI_PENDING_OFFSET (MENTRY_HLS_OFFSET + REGBYTES)

#ifdef __riscv_flen
# define SOFT_FLOAT_CONTEXT_SIZE 0
#else
# define SOFT_FLOAT_CONTEXT_SIZE (8 * 32)
#endif
#define HLS_SIZE 64
#define INTEGER_CONTEXT_SIZE (32 * REGBYTES)






#endif
