#include <asm.h>
#include <mm.h>
#include <serial.h>
#include <system.h>

uintptr *freemap_4k, *freemap_2m;
extern char kernel_end[];

#define BIT64_MASK(n) ((1ULL << (n)) - 1)

#define PG_P (0x1ULL)        // Present
#define PG_RW (0x1ULL << 1)  // Read/Write (0=Read-Only, 1=Read/Write)
#define PG_US (0x1ULL << 2)  // User/Supervisor (0=Supervisor, 1=User)
#define PG_PS (1ULL << 7)    // Page size (1=PageFrame, 0=PagePointer)

void memory_init() {
    uint64 bitmask = (BIT64_MASK(MAX_PADDR_BITS) & ~BIT64_MASK(12));

    uint64 cr3 = lcr3();
    uint64 page_1gb = 0, page_2mb = 0, page_4kb = 0;

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
                if (pd[k] & PG_PS) {
                    page_2mb++;
                    continue;
                }
                uint64 *pt = (uint64 *)(pd[k] & bitmask);
                for (uint64 l = 0; l < 512; l++) {
                    if ((pt[k] & 1) == 0) {
                        continue;
                    }
                    page_4kb++;
                }
            }
        }
    }
    kprintf("page_1gb: %x\n", page_1gb);
    kprintf("page_2mb: %x\n", page_2mb);
    kprintf("page_4kb: %x\n", page_4kb);
}