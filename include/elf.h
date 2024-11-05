#pragma once
#include <types.h>

typedef uint16 Elf64_Half;
typedef uint32 Elf64_Word;
typedef uint64 Elf64_Xword;
typedef uint64 Elf64_Addr;
typedef uint64 Elf64_Off;

#define ELF_MAGIC 0x464C457FU /* "\x7FELF" in little endian */

typedef struct {
    uint32 magic;           /* must equal ELF_MAGIC */
    uint8 e_ident[12];      /* Magic number and other info */
    Elf64_Half e_type;      /* Object file type */
    Elf64_Half e_machine;   /* Architecture */
    Elf64_Word e_version;   /* Object file version */
    Elf64_Addr e_entry;     /* Entry point virtual address */
    Elf64_Off e_phoff;      /* Program header table file offset */
    Elf64_Off e_shoff;      /* Section header table file offset */
    Elf64_Word e_flags;     /* Processor-specific flags */
    Elf64_Half e_ehsize;    /* ELF header size in bytes */
    Elf64_Half e_phentsize; /* Program header table entry size */
    Elf64_Half e_phnum;     /* Program header table entry count */
    Elf64_Half e_shentsize; /* Section header table entry size */
    Elf64_Half e_shnum;     /* Section header table entry count */
    Elf64_Half e_shstrndx;  /* Section header string table index */
} Elf64_Ehdr;

// Values for Proghdr type
#define ELF_PROG_LOAD 1

typedef struct {
    Elf64_Word p_type;    /* Segment type */
    Elf64_Word p_flags;   /* Segment flags */
    Elf64_Off p_offset;   /* Segment file offset */
    Elf64_Addr p_vaddr;   /* Segment virtual address */
    Elf64_Addr p_paddr;   /* Segment physical address */
    Elf64_Xword p_filesz; /* Segment size in file */
    Elf64_Xword p_memsz;  /* Segment size in memory */
    Elf64_Xword p_align;  /* Segment alignment */
} Elf64_Phdr;
