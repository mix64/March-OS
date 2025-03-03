#include <kernel.h>
#include <mm.h>
#include <x86/asm.h>
#include <x86/mm.h>

static pte_t *pgdir;

void create_pte(pte_t *pte, uint64 flag);

void vm_init() {
    pgdir = (pte_t *)pmalloc(PM_4K);
    for (uintptr va = 0; va < GiB(4); va += MiB(2)) {
        pte_t *r = walk_pgdir(va, TRUE, TRUE);
        if (r == NULL) {
            panic("vm_init: create page failed %x\n", va);
        }
    }
    debugf("[vm] Mapped 0 - %d MiB\n", GiB(4) / MiB(1));
    scr3(pgdir);
}

// create: create page if page not present
// large: create 2MB page
pte_t *walk_pgdir(uintptr va, bool create, bool large) {
    /*
     * 4-level page table
     * PML4 -> PDPT -> PD -> PT
     * 9-bit index for each level
     * 4KB page: 12-bit offset
     * 2MB page: 21-bit offset
     */
    uint64 flag = PG_P | PG_RW;
    if (va >= USER_ADDR_START) {
        flag |= PG_US;
    }

    // PML4
    pte_t *pml4 = (pte_t *)(ROUNDDOWN((uintptr)pgdir, KiB(4)));
    if ((pml4[PML4_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        pml4[PML4_IDX(va)] = (uintptr)pmalloc(PM_4K) | flag;
    }

    // PDPT
    pte_t *pdpt = (pte_t *)(pml4[PML4_IDX(va)] & ~BIT64_MASK(12));
    if ((pdpt[PDPT_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        pdpt[PDPT_IDX(va)] = (uintptr)pmalloc(PM_4K) | flag;
    }

    // PD
    pte_t *pd = (pte_t *)(pdpt[PDPT_IDX(va)] & ~BIT64_MASK(12));
    if ((pd[PD_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        if (large) {
            // direct mapping for kernel space
            if (va < USER_ADDR_START) {
                pd[PD_IDX(va)] = ROUNDDOWN(va, MiB(2)) | flag | PG_PS;
            } else {
                pd[PD_IDX(va)] = (uintptr)pmalloc(PM_4K) | flag | PG_PS;
            }
        } else {
            pd[PD_IDX(va)] = (uintptr)pmalloc(PM_4K) | flag;
        }
    }
    if (pd[PD_IDX(va)] & PG_PS) {
        return &pd[PD_IDX(va)];  // 2MB page
    }

    // PT
    pte_t *pt = (pte_t *)(pd[PD_IDX(va)] & ~BIT64_MASK(12));
    if ((pt[PT_IDX(va)] & PG_P) == 0) {
        if (!create) {
            return NULL;
        }
        // direct mapping for kernel space
        if (va < USER_ADDR_START) {
            pt[PT_IDX(va)] = ROUNDDOWN(va, KiB(4)) | flag;
        } else {
            pt[PT_IDX(va)] = (uintptr)pmalloc(PM_4K) | flag;
        }
    }
    return &pt[PT_IDX(va)];  // 4KB page
}

void setflag(pte_t *pte, uint64 flag) { *pte |= flag; }

void switch_uvm(pte_t upml4) {
    pte_t *pml4 = (pte_t *)(ROUNDDOWN((uintptr)pgdir, KiB(4)));
    pml4[PML4_IDX(USER_ADDR_START)] = upml4;
}

void free_uvm(pte_t upml4) {
    pte_t *pml4 = (pte_t *)(ROUNDDOWN((uintptr)pgdir, KiB(4)));
    if ((pml4[PML4_IDX(USER_ADDR_START)] & PG_P) == 0) {
        panic("free_uvm: pml4[2] not present\n");
    }
    pte_t *pdpt = (pte_t *)(pml4[PML4_IDX(USER_ADDR_START)] & ~BIT64_MASK(12));
    for (uint64 i = 0; i < 512; i++) {
        if (pdpt[i] & PG_P) {
            pte_t *pd = (pte_t *)(pdpt[i] & ~BIT64_MASK(12));
            for (uint64 j = 0; j < 512; j++) {
                if (pd[j] & PG_P) {
                    pte_t *pt = (pte_t *)(pd[j] & ~BIT64_MASK(12));
                    for (uint64 k = 0; k < 512; k++) {
                        if (pt[k] & PG_P) {
                            pmfree((void *)(pt[k] & ~BIT64_MASK(12)), PM_4K);
                        }
                    }
                    pmfree(pt, PM_4K);
                }
            }
            pmfree(pd, PM_4K);
        }
    }
    pmfree(pdpt, PM_4K);
    pml4[PML4_IDX(USER_ADDR_START)] = 0;
}
