#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <mm/slab.h>

SLAB slabs[SLAB_NUM];

void *find_free_slab(enum SLAB_IDX idx);
enum SLAB_IDX _size_to_slab_idx(uint64 size);
void slab_tests();

void slab_init() {
    for (int i = 0; i < SLAB_NUM; i++) {
        if (i > SLAB_4K) {
            slabs[i].address = (uintptr)pmalloc(PM_512K);
        } else {
            slabs[i].address = (uintptr)pmalloc(PM_4K);
        }
        slabs[i].bitmap = 0;
        slabs[i].next = NULL;
    }
#ifdef __DEBUG__
    slab_tests();
#endif
}

void *kmalloc(uint64 size) {
    if (size <= 64) {
        void *addr = find_free_slab(SLAB_64);
        if (addr != NULL) {
            memset(addr, 0, 64);
            return addr;
        }
        // No free space, allocate new slab
        // 1st sector used by SLAB struct
        SLAB *last = &slabs[SLAB_64];
        for (; last->next != NULL; last = last->next);
        SLAB *new = (SLAB *)pmalloc(PM_4K);
        new->address = (uintptr) new;
        new->bitmap = 0b11;
        last->next = new;
        return (void *)(new->address + 64);
    }

    enum SLAB_IDX idx = _size_to_slab_idx(size);
    void *addr = find_free_slab(idx);
    if (addr != NULL) {
        memset(addr, 0, SLAB_MIN_SIZE << idx);
        return addr;
    }

    // No free space, allocate new slab
    SLAB *entry = &slabs[idx];
    for (; entry->next != NULL; entry = entry->next);
    SLAB *new = (SLAB *)kmalloc(sizeof(SLAB));
    if (idx > SLAB_4K) {
        new->address = (uintptr)pmalloc(PM_512K);
    } else {
        new->address = (uintptr)pmalloc(PM_4K);
    }
    new->bitmap = 0b1;
    entry->next = new;
    return (void *)(new->address);
}

void *find_free_slab(enum SLAB_IDX idx) {
    if (idx > SLAB_NUM) {
        panic("find_free_slab\n");
    }
    SLAB *entry = &slabs[idx];
    uint64 size = SLAB_MIN_SIZE << idx;
    uint64 pages = size > KiB(4) ? (PM_512K / size) : (PM_4K / size);
    while (entry != NULL) {
        if (entry->bitmap != 0xFFFFFFFFFFFFFFFF) {
            for (int i = 0; i < pages; i++) {
                if ((entry->bitmap & (1ULL << i)) == 0) {
                    entry->bitmap |= (1ULL << i);
                    return (void *)(entry->address + i * size);
                }
            }
        }
        entry = entry->next;
    }
    return NULL;
}

void kmfree(void *addr_p) {
    if (addr_p == NULL) {
        panic("kmfree: addr is NULL\n");
    }

    uintptr addr = (uintptr)addr_p;
    SLAB *entry, *prev;
    uint64 page_size, slab_size;

    // Find entry
    for (int i = 0; i < SLAB_NUM; i++) {
        entry = &slabs[i];
        prev = NULL;
        page_size = i > SLAB_4K ? PM_512K : PM_4K;
        slab_size = SLAB_MIN_SIZE << i;
        while (entry != NULL) {
            if (entry->address + page_size > addr && entry->address <= addr) {
                goto found_entry;
            }
            prev = entry;
            entry = entry->next;
        }
    }

found_entry:
    if (entry == NULL) {
        panic("kmfree: slab addr %x not found\n", addr);
    }

    // Free slab sector
    int idx = (addr - entry->address) / slab_size;
    if (addr % slab_size != 0 || idx < 0 || idx >= (page_size / slab_size)) {
        panic("kmfree: invalid addr %x\n", addr);
    }
    entry->bitmap &= ~(1ULL << idx);

    // Free slab if all sectors are free
    if (entry->bitmap == 0 && prev != NULL) {
        prev->next = entry->next;
        kmfree((void *)entry);
    }

    // Free slab64 if all sectors are free
    if (entry->bitmap == 0b1 && slab_size == SLAB_MIN_SIZE && prev != NULL) {
        prev->next = entry->next;
        if (page_size > KiB(4)) {
            pmfree((void *)entry, PM_512K);
        } else {
            pmfree((void *)entry, PM_4K);
        }
    }
}

void slab_tests() {
    debugf("[slab] start slab_tests: ");

    void *addr[65];
    void *addr2[64];
    SLAB *slab64 = &slabs[SLAB_64];
    SLAB *slab128 = &slabs[SLAB_128];

    for (int i = 0; i < 65; i++) {
        addr[i] = kmalloc(64);
    }
    /* expected:
        [0] bitmap:0bFFFFFFFF_FFFFFFFF, next->[1]
        [1] bitmap:0b11, next->null
    */
    if (slab64->bitmap != 0xFFFFFFFFFFFFFFFF || slab64->next == NULL) {
        goto failed;
    }
    if (slab64->next->bitmap != 0b11 || slab64->next->next != NULL) {
        goto failed;
    }
    debugf("1 ");

    for (int i = 0; i < 65; i += 2) {
        kmfree(addr[i]);
    }
    /* expected:
        [0] bitmap:0xAAAAAAAA_AAAAAAAA, next->null
    */
    if (slab64->bitmap != 0xAAAAAAAAAAAAAAAA || slab64->next != NULL) {
        goto failed;
    }
    debugf("2 ");

    for (int i = 0; i < 64; i++) {
        addr2[i] = kmalloc(64);
    }
    /* expected:
        [0] bitmap:0xFFFFFFFF_FFFFFFFF, next->[1]
        [1] bitmap:0x00000001_FFFFFFFF, next->null
    */
    if (slab64->bitmap != 0xFFFFFFFFFFFFFFFF || slab64->next == NULL) {
        goto failed;
    }
    if (slab64->next->bitmap != 0x1FFFFFFFF || slab64->next->next != NULL) {
        goto failed;
    }
    debugf("3 ");

    for (int i = 1; i < 65; i += 2) {
        kmfree(addr[i]);
    }
    for (int i = 0; i < 64; i++) {
        kmfree(addr2[i]);
    }
    /* expected:
        [0] bitmap:0, next->null, prev->null
    */
    if (slab64->bitmap != 0 || slab64->next != NULL) {
        goto failed;
    }
    debugf("4 ");

    // test slab128
    for (int i = 0; i < 33; i++) {
        addr[i] = kmalloc(128);
    }
    /* expected slab64:
        [0] bitmap:0b1, next->null
       expected slab128:
        [0] bitmap:0xFFFFFFFF, next->[1]
        [1] bitmap:0b1, next->null
    */
    if ((slab64->bitmap != 0b1 || slab64->next != NULL) ||
        (slab128->bitmap != 0xFFFFFFFF || slab128->next == NULL) ||
        (slab128->next->bitmap != 0b1 || slab128->next->next != NULL)) {
        goto failed;
    }
    debugf("5 ");

    for (int i = 0; i < 33; i += 2) {
        kmfree(addr[i]);
    }
    /* expected slab64:
        [0] bitmap:0b0, next->null
       expected slab128:
        [0] bitmap:0xAAAAAAAA, next->null
    */
    if ((slab64->bitmap != 0 || slab64->next != NULL) ||
        (slab128->bitmap != 0xAAAAAAAA || slab128->next != NULL)) {
        goto failed;
    }
    debugf("6 ");

    for (int i = 1; i < 33; i += 2) {
        kmfree(addr[i]);
    }
    if ((slab64->bitmap != 0 || slab64->next != NULL) ||
        (slab128->bitmap != 0 || slab128->next != NULL)) {
        goto failed;
    }
    debugf("7\n");
    debugf("[slab] finish slab_tests\n");
    return;

failed:
    while (slab64 != NULL) {
        debugf("Dump slab64: %x\n", slab64);
        debugf("             address %x\n", slab64->address);
        debugf("             bitmap %x\n", slab64->bitmap);
        debugf("             next %x\n", slab64->next);
        slab64 = slab64->next;
    }
    while (slab128 != NULL) {
        debugf("Dump slab128: %x\n", slab128);
        debugf("             address %x\n", slab128->address);
        debugf("             bitmap %x\n", slab128->bitmap);
        debugf("             next %x\n", slab128->next);
        slab128 = slab128->next;
    }

    panic("slab_tests failed\n");
}

enum SLAB_IDX _size_to_slab_idx(uint64 size) {
    if (size <= 64) {
        return SLAB_64;
    } else if (size <= 128) {
        return SLAB_128;
    } else if (size <= 256) {
        return SLAB_256;
    } else if (size <= 512) {
        return SLAB_512;
    } else if (size <= KiB(1)) {
        return SLAB_1K;
    } else if (size <= KiB(2)) {
        return SLAB_2K;
    } else if (size <= KiB(4)) {
        return SLAB_4K;
    } else if (size <= KiB(8)) {
        return SLAB_8K;
    } else if (size <= KiB(16)) {
        return SLAB_16K;
    } else if (size <= KiB(32)) {
        return SLAB_32K;
    } else if (size <= KiB(64)) {
        return SLAB_64K;
    } else if (size <= KiB(128)) {
        return SLAB_128K;
    } else if (size <= KiB(256)) {
        return SLAB_256K;
    } else if (size <= KiB(512)) {
        return SLAB_512K;
    } else {
        panic("kmalloc: size %d not supported\n", size);
    }
}
