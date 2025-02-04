#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <system.h>
#include <x86/asm.h>

uintptr *pmmap[PMMAP_NUM];
extern char kernel_end[];

void pm_dump();
uint8 _pm_size_to_idx(enum PMMAP size);

void *pmalloc(enum PMMAP size) {
    uint8 idx = _pm_size_to_idx(size);
    void *addr;

    if (pmmap[idx] == NULL) {
        if (size == PM_64M) {
            panic("pmalloc: out of memory");
        }
        if (size == PM_512K) {
            pmmap[idx] = pmalloc(PM_64M);
            for (int i = 0; i < PM_64M / PM_512K; i++) {
                addr = (void *)((uintptr)pmmap[idx] + PM_64M - PM_512K * i);
                addr -= PM_512K;
                pmfree(addr, PM_512K);
            }
        } else if (size == PM_4K) {
            pmmap[idx] = pmalloc(PM_512K);
            for (int i = 0; i < PM_512K / PM_4K; i++) {
                addr = (void *)((uintptr)pmmap[idx] + PM_512K - PM_4K * i);
                addr -= PM_4K;
                pmfree(addr, PM_4K);
            }
        }
    }

    addr = (void *)pmmap[idx];
    pmmap[idx] = (uintptr *)*pmmap[idx];
    memset(addr, 0, size);
    return addr;
}

void pmfree(void *addr, enum PMMAP size) {
    uint8 idx = _pm_size_to_idx(size);
    if (addr == NULL || (uintptr)addr < (uintptr)kernel_end ||
        (uintptr)addr >= SYSTEM.memtotal) {
        panic("pmfree: invalid address %x", addr);
    }
    *(uintptr *)addr = (uintptr)pmmap[idx];
    pmmap[idx] = addr;
}

void pm_dump() {
    for (int i = 0; i < PMMAP_NUM; i++) {
        uintptr *map = pmmap[i];
        uint64 count = 0;
        if (map == NULL) {
            continue;
        }
        for (uintptr *p = map; p != NULL; p = (uintptr *)*p) {
            count++;
        }
        if (i == 0) {
            kprintf("[pm] pmmap[4K]: %d\n", count);
        } else if (i == 1) {
            kprintf("[pm] pmmap[512K]: %d\n", count);
        } else if (i == 2) {
            kprintf("[pm] pmmap[64M]: %d\n", count);
        }
    }
}

uint8 _pm_size_to_idx(enum PMMAP size) {
    if (size == PM_4K) {
        return 0;
    } else if (size == PM_512K) {
        return 1;
    } else if (size == PM_64M) {
        return 2;
    } else {
        panic("pm: invalid size %d", size);
    }
}

void pm_init() {
    uint64 mask_lo12 = (BIT64_MASK(MAX_PADDR_BITS) & ~BIT64_MASK(12));
    uint64 mask_lo21 = (BIT64_MASK(MAX_PADDR_BITS) & ~BIT64_MASK(21));
    uint64 allocatable_max = 0;
    uint64 cr3 = lcr3();

    uintptr *pml4 = (uintptr *)(cr3 & mask_lo12);
    for (uint16 i = 0; i < 512; i++) {
        if ((pml4[i] & PG_P) == 0) {
            continue;
        }
        uintptr *pdpt = (uintptr *)(pml4[i] & mask_lo12);
        for (uint16 j = 0; j < 512; j++) {
            if ((pdpt[j] & PG_P) == 0) {
                continue;
            }
            uintptr *pd = (uintptr *)(pdpt[j] & mask_lo12);
            for (uint16 k = 0; k < 512; k++) {
                if ((pd[k] & PG_P) == 0) {
                    continue;
                }
                if ((pd[k] & PG_RW) == 0) {
                    allocatable_max = pd[k] & mask_lo21;
                    goto out;
                }
                if (pd[k] & PG_PS) {
                    // 2MB page
                    continue;
                }
                uintptr *pt = (uintptr *)(pd[k] & mask_lo12);
                for (uint16 l = 0; l < 512; l++) {
                    if ((pt[l] & PG_P) == 0) {
                        continue;
                    }
                    if ((pt[l] & PG_RW) == 0) {
                        // 4KB page
                        allocatable_max = pt[l] & mask_lo12;
                        goto out;
                    }
                }
            }
        }
    }

out:
    for (uint64 addr = (allocatable_max - PAGE_SIZE);
         addr >= (uint64)kernel_end; addr -= PAGE_SIZE) {
        pmfree((void *)addr, KiB(4));
    }
    pm_dump();
}