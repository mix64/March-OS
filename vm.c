#include <kernel.h>
#include <mm.h>
#include <x86/asm.h>

static uintptr *pgdir;

void create_pte(pte_t *pte, uint64 flag);

void vm_init() {
    pgdir = (uintptr *)kalloc();
    for (uintptr va = 0; va < (1ULL << MAX_PADDR_BITS); va += MiB(2)) {
        pte_t *r = walk_pgdir(va, TRUE, TRUE);
        if (r == NULL) {
            panic("vm_init: create page failed %x\n", va);
        }
    }
    void *addr = kalloc();
    debugf("[vm] Mapped 0 - %x\n", 1ULL << MAX_PADDR_BITS);
    debugf("[vm] pgdir size: %x\n", (uintptr)addr - (uintptr)pgdir);
    kfree(addr);
    scr3(pgdir);
}

// create: create page if page not present
// large: create 2MB page
pte_t *walk_pgdir(uintptr va, bool create, bool large) {
    pte_t *pml4 = (pte_t *)(ROUNDDOWN((uintptr)pgdir, KiB(4)));
    if ((pml4[PML4_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        create_pte(&pml4[PML4_IDX(va)], PG_P | PG_RW);
    }
    pte_t *pdpt = (pte_t *)(pml4[PML4_IDX(va)] & ~BIT64_MASK(12));
    if ((pdpt[PDPT_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        create_pte(&pdpt[PDPT_IDX(va)], PG_P | PG_RW);
    }
    pte_t *pd = (pte_t *)(pdpt[PDPT_IDX(va)] & ~BIT64_MASK(12));
    if ((pd[PD_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        if (large) {
            pd[PD_IDX(va)] = ROUNDDOWN(va, MiB(2)) | PG_P | PG_RW | PG_PS;
        } else {
            create_pte(&pd[PD_IDX(va)], PG_P | PG_RW);
        }
    }
    if (pd[PD_IDX(va)] & PG_PS) {
        return &pd[PD_IDX(va)];  // 2MB page
    }
    pte_t *pt = (pte_t *)(pd[PD_IDX(va)] & ~BIT64_MASK(12));
    if ((pt[PT_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        pt[PT_IDX(va)] = ROUNDDOWN(va, KiB(4)) | PG_P | PG_RW;
    }
    return &pt[PT_IDX(va)];  // 4KB page
}

void setflag(pte_t *pte, uint64 flag) { *pte |= flag; }

void create_pte(pte_t *pte, uint64 flag) { *pte = (uintptr)kalloc() | flag; }