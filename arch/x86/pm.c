
#include <kernel.h>
#include <mm.h>
#include <x86/asm.h>
#include <x86/cpuid.h>
#include <x86/mm.h>

void pm_init() {
    uint64 max_paddr = CPUID_8000_0008_EAX & 0xFF;
    uint64 mask_lo12 = (BIT64_MASK(max_paddr) & ~BIT64_MASK(12));
    uint64 mask_lo21 = (BIT64_MASK(max_paddr) & ~BIT64_MASK(21));
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
    debugf("[pm] kernel_end: %x\n", kernel_end);
    debugf("[pm] allocatable_max: %x\n", allocatable_max);
    while (allocatable_max >= (uint64)kernel_end + PM_64M) {
        allocatable_max -= PM_64M;
        pmfree((void *)allocatable_max, PM_64M);
    }
    while (allocatable_max >= (uint64)kernel_end + PM_512K) {
        allocatable_max -= PM_512K;
        pmfree((void *)allocatable_max, PM_512K);
    }
    while (allocatable_max >= (uint64)kernel_end + PM_4K) {
        allocatable_max -= PM_4K;
        pmfree((void *)allocatable_max, PM_4K);
    }
    pm_dump();
}