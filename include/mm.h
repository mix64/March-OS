#pragma once
#include <types.h>

#define PAGE_SIZE KiB(4ULL)

#define PML4_IDX(va) (((uint64)(va) >> 39) & 0x1FF)
#define PDPT_IDX(va) (((uint64)(va) >> 30) & 0x1FF)
#define PD_IDX(va) (((uint64)(va) >> 21) & 0x1FF)
#define PT_IDX(va) (((uint64)(va) >> 12) & 0x1FF)

#define PG_P (1ULL)          // Present
#define PG_RW (1ULL << 1)    // Read/Write (0=Read-Only, 1=Read/Write)
#define PG_US (1ULL << 2)    // User/Supervisor (0=Supervisor, 1=User)
#define PG_PWT (1ULL << 3)   // Page-Level Write-Through
#define PG_PCD (1ULL << 4)   // Page-Level Cache Disable
#define PG_PS (1ULL << 7)    // Page size (1=PageFrame, 0=PagePointer)
#define PD_PAT (1ULL << 12)  // Page Attribute Table

// mm.c
void mm_init();
void *kalloc();
void kfree(void *addr);

// vm.c
void vm_init();
pte_t *walk_pgdir(uintptr va, bool create, bool large);
void setflag(pte_t *pte, uint64 flag);