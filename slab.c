#include <mm.h>
#include <mm/slab.h>
#include <serial.h>
#include <types.h>

SLAB slabs[SLAB_NUM];

void *find_free_slab(enum SLAB_IDX idx);
void slab_tests();

void slab_init() {
    for (int i = 0; i < SLAB_NUM; i++) {
        slabs[i].address = (uintptr)kalloc();
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
            return addr;
        }
        // No free space, allocate new slab
        // 1st sector used by SLAB struct
        SLAB *last = &slabs[SLAB_64];
        for (; last->next != NULL; last = last->next);
        SLAB *new = (SLAB *)kalloc();
        new->address = (uintptr) new;
        new->bitmap = 0b11;
        last->next = new;
        return (void *)(new->address + 64);
    }

    enum SLAB_IDX idx;
    if (size <= 128) {
        idx = SLAB_128;
    } else if (size <= 256) {
        idx = SLAB_256;
    } else if (size <= 512) {
        idx = SLAB_512;
    } else if (size <= 1024) {
        idx = SLAB_1024;
    } else if (size <= 2048) {
        idx = SLAB_2048;
    } else if (size <= 4096) {
        idx = SLAB_4096;
    } else {
        panic("kmalloc: size %d not supported\n", size);
    }

    void *addr = find_free_slab(idx);
    if (addr != NULL) {
        return addr;
    }

    // No free space, allocate new slab
    SLAB *entry = &slabs[idx];
    for (; entry->next != NULL; entry = entry->next);
    SLAB *new = (SLAB *)kmalloc(sizeof(SLAB));
    new->address = (uintptr)kalloc();
    new->bitmap = 0b1;
    entry->next = new;
    return (void *)(new->address + (SLAB_MIN_SIZE << idx));
}

void *find_free_slab(enum SLAB_IDX idx) {
    if (idx > SLAB_NUM) {
        panic("find_free_slab\n");
    }
    SLAB *entry = &slabs[idx];
    uint64 size = SLAB_MIN_SIZE << idx;
    while (entry != NULL) {
        if (entry->bitmap != 0xFFFFFFFFFFFFFFFF) {
            for (int i = 0; i < (PAGE_SIZE / size); i++) {
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
    uintptr base = ROUNDDOWN(addr, PAGE_SIZE);
    SLAB *entry, *prev;
    uint64 size;

    // Find entry
    for (int i = 0; i < SLAB_NUM; i++) {
        entry = &slabs[i];
        prev = NULL;
        while (entry != NULL) {
            if (entry->address == base) {
                size = SLAB_MIN_SIZE << i;
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
    int idx = (addr - base) / size;
    if (addr % size != 0 || idx < 0 || idx >= size) {
        panic("kmfree: invalid addr %x\n", addr);
    }
    entry->bitmap &= ~(1ULL << idx);

    // Free slab if all sectors are free
    if ((entry->bitmap == 0 && prev != NULL) ||
        (entry->bitmap == 0b1 && size == SLAB_MIN_SIZE && prev != NULL)) {
        prev->next = entry->next;
        kfree((void *)entry);
    }
}

void slab_tests() {
    debugf("[slab] start slab_tests\n");

    void *addr[65];
    void *addr2[64];
    SLAB *slab64 = &slabs[SLAB_64];

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
    debugf("       pass test1\n");

    for (int i = 0; i < 65; i += 2) {
        kmfree(addr[i]);
    }
    /* expected:
        [0] bitmap:0xAAAAAAAA_AAAAAAAA, next->null
    */
    if (slab64->bitmap != 0xAAAAAAAAAAAAAAAA || slab64->next != NULL) {
        goto failed;
    }
    debugf("       pass test2\n");

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
    debugf("       pass test3\n");

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
    debugf("       pass test4\n");
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
    panic("slab_tests failed\n");
}
