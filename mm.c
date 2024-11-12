#include <asm.h>
#include <mm.h>
#include <serial.h>
#include <system.h>

uintptr *freemap;
extern char kernel_end[];

#define BIT64_MASK(n) ((1ULL << (n)) - 1)

#define PG_P (0x1ULL)        // Present
#define PG_RW (0x1ULL << 1)  // Read/Write (0=Read-Only, 1=Read/Write)
#define PG_US (0x1ULL << 2)  // User/Supervisor (0=Supervisor, 1=User)
#define PG_PS (1ULL << 7)    // Page size (1=PageFrame, 0=PagePointer)

#define KiB(x) (x << 10)
#define MiB(x) (x << 20)
#define GiB(x) (x << 30)
#define PAGE_SIZE KiB(4ULL)

void kfree(void *addr) {
    if (addr == NULL || (uintptr)addr < (uintptr)kernel_end ||
        (uintptr)addr >= SYSTEM.memtotal) {
        panic("kfree: invalid address %x", addr);
    }
    memset(addr, 0, PAGE_SIZE);
    *(uintptr *)addr = (uintptr)freemap;
    freemap = addr;
}

void memory_init() {
    uint64 bitmask = (BIT64_MASK(MAX_PADDR_BITS) & ~BIT64_MASK(12));

    uint64 cr3 = lcr3();
    uint64 page_1gb = 0, page_2mb = 0, page_4kb = 0;
    uint64 allocatable_max = 0;

    uintptr *pml4 = (uintptr *)(cr3 & bitmask);
    for (uint64 i = 0; i < 512; i++) {
        if ((pml4[i] & PG_P) == 0) {
            continue;
        }
        if (pml4[i] & PG_PS) {
            page_1gb++;
            continue;
        }
        uintptr *pdpt = (uintptr *)(pml4[i] & bitmask);
        for (uint64 j = 0; j < 512; j++) {
            if ((pdpt[j] & PG_P) == 0) {
                continue;
            }
            uint64 *pd = (uint64 *)(pdpt[j] & bitmask);
            for (uint64 k = 0; k < 512; k++) {
                if ((pd[k] & PG_P) == 0) {
                    continue;
                }
                if ((pd[k] & PG_RW) == 0) {
                    if (allocatable_max == 0) {
                        allocatable_max = pd[k] & (BIT64_MASK(MAX_PADDR_BITS) &
                                                   ~BIT64_MASK(20));
                    }
                }
                if (pd[k] & PG_PS) {
                    page_2mb++;
                    continue;
                }
                uint64 *pt = (uint64 *)(pd[k] & bitmask);
                for (uint64 l = 0; l < 512; l++) {
                    if ((pt[k] & PG_P) == 0) {
                        continue;
                    }
                    if ((pt[l] & PG_RW) == 0) {
                        if (allocatable_max == 0) {
                            allocatable_max = pt[l] & bitmask;
                        }
                    }
                    page_4kb++;
                }
            }
        }
    }
    kprintf("1GB Page: %x\n", page_1gb);
    kprintf("2MB Page: %x\n", page_2mb);
    kprintf("4KB Page: %x\n", page_4kb);
    kprintf("Allocatable Max Address: %x\n", allocatable_max);

    for (uint64 addr = (allocatable_max - PAGE_SIZE);
         addr >= (uint64)kernel_end; addr -= PAGE_SIZE) {
        kfree((void *)addr);
    }
    kprintf("freemap: %x\n", freemap);
}
