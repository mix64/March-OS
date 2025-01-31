#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <system.h>
#include <x86/asm.h>

uintptr *freemap;
extern char kernel_end[];

void *kalloc() {
    if (freemap == NULL) {
        panic("kalloc: out of memory");
    }
    void *addr = (void *)freemap;
    freemap = (uintptr *)*freemap;
    memset(addr, 0, PAGE_SIZE);
    return addr;
}

void kfree(void *addr) {
    if (addr == NULL || (uintptr)addr < (uintptr)kernel_end ||
        (uintptr)addr >= SYSTEM.memtotal) {
        panic("kfree: invalid address %x", addr);
    }
    *(uintptr *)addr = (uintptr)freemap;
    freemap = addr;
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
        kfree((void *)addr);
    }
    kprintf("[pm] freemap: %x\n", freemap);
}
