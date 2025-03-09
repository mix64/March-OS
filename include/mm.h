#pragma once
#include <common.h>
#include <proc.h>
#include <types.h>

#define PAGE_SIZE KiB(4ULL)
#define USER_ADDR_START 0x10000000000

// kernel.ls
extern char kernel_end[];

// init.c
void mm_init();

// pm.c
#define PMMAP_NUM 3
enum PMMAP {
    PM_4K = PAGE_SIZE,
    PM_512K = KiB(512ULL),
    PM_64M = MiB(64ULL),
};
void *pmalloc(enum PMMAP size);
void pmfree(void *addr, enum PMMAP size);
void pm_dump();

// vm.c
pte_t *walk_pgdir(uintptr va, bool create, bool large);
void setflag(pte_t *pte, uint64 flag);
void switch_uvm(proc_t *proc);

// slab.c
void *kmalloc(uint64 size);
void kmfree(void *addr);
