#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <system.h>

uintptr *pmmap[PMMAP_NUM];

uint8 _pm_size_to_idx(enum PMMAP size);

void *pmalloc(enum PMMAP size) {
    uint8 idx = _pm_size_to_idx(size);
    uintptr addr;

    if (pmmap[idx] == NULL) {
        if (size == PM_64M) {
            panic("pmalloc: out of memory");
        }
        if (size == PM_512K) {
            addr = (uintptr)pmalloc(PM_64M) + PM_64M - PM_512K;
            for (int i = 0; i < 128; i++) {  // 128 = 64M / 512K
                pmfree((void *)addr, PM_512K);
                addr -= PM_512K;
            }
        } else if (size == PM_4K) {
            addr = (uintptr)pmalloc(PM_512K) + PM_512K - PM_4K;
            for (int i = 0; i < 128; i++) {  // 128 = 512K / 4K
                pmfree((void *)addr, PM_4K);
                addr -= PM_4K;
            }
        }
    }

    addr = (uintptr)pmmap[idx];
    pmmap[idx] = (uintptr *)*pmmap[idx];
    memset((void *)addr, 0, size);
    return (void *)addr;
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
