#include <mm.h>
#include <serial.h>
#include <types.h>

struct slab64_info {
    uint64 bitmap;
    struct slab64_info *prev;
    struct slab64_info *next;
};

static struct slab64_info *slab64;

void slab_tests();

void slab_init() {
    slab64 = (struct slab64_info *)kalloc();
    slab64->bitmap = 0b1;
    slab64->prev = NULL;
    slab64->next = NULL;
#ifdef __DEBUG__
    slab_tests();
#endif
}

void *kmalloc(uint64 size) {
    if (size <= 64) {
        if (slab64 == NULL) {
            panic("kmalloc: slab64 is NULL\n");
        }

        // Find free space in slab64
        struct slab64_info *entry = slab64;
        while (entry != NULL) {
            for (int i = 1; i < 64; i++) {
                if ((entry->bitmap & (1ULL << i)) == 0) {
                    entry->bitmap |= (1ULL << i);
                    return (void *)((uintptr)(entry) + i * 64);
                }
            }
            entry = entry->next;
        }

        // No free space, allocate new slab64
        struct slab64_info *new = (struct slab64_info *)kalloc();
        new->bitmap = 0b11;
        new->next = slab64;
        slab64->prev = new;
        slab64 = new;

        return (void *)((uintptr)(slab64) + 64);
    }
    // TODO: Implement kmalloc for other size
    return NULL;
}

void kmfree(void *addr_p) {
    if (addr_p == NULL) {
        panic("kmfree: addr is NULL\n");
    }

    uintptr addr = (uintptr)addr_p;
    uintptr base = ROUNDDOWN(addr, PAGE_SIZE);
    struct slab64_info *entry = slab64;

    while (entry != NULL) {
        if ((uintptr)entry == base) {
            int idx = (addr - base) / 64;
            if (addr % 64 != 0 || idx < 0 || idx > 63) {
                panic("kmfree: invalid addr %x\n", addr);
            }
            entry->bitmap &= ~(1ULL << idx);
            if (entry->bitmap == 0b1) {
                if (entry->prev != NULL) {
                    entry->prev->next = entry->next;
                }
                if (entry->next != NULL) {
                    entry->next->prev = entry->prev;
                }
                // entry is head
                if (entry == slab64) {
                    // but not the only one
                    if (entry->next != NULL) {
                        slab64 = entry->next;
                    } else {
                        // the only one, not free
                        return;
                    }
                }
                kfree((void *)entry);
            }
            return;
        }
        entry = entry->next;
    }
    panic("kmfree: slab addr %x not found\n", addr);
}

void slab_tests() {
    debugf("[slab] start slab_tests\n");

    void *addr[64];
    void *addr2[64];
    struct slab64_info *entry = slab64;

    for (int i = 0; i < 64; i++) {
        addr[i] = kmalloc(64);
    }
    /* expected:
        [0] bitmap:0b11, next->[1], prev->null
        [1] bitmap:0xFFFFFFFF_FFFFFFFF, next->null, prev->[0]
    */
    if (slab64->bitmap != 0b11 || slab64->next == NULL ||
        slab64->prev != NULL) {
        goto failed;
    }
    entry = slab64->next;
    if (entry->bitmap != 0xFFFFFFFFFFFFFFFF || entry->next != NULL ||
        entry->prev != slab64) {
        goto failed;
    }
    debugf("       pass test1\n");

    for (int i = 0; i < 64; i += 2) {
        kmfree(addr[i]);
    }
    /* expected:
        [0] bitmap:0b11, next->[1], prev->null
        [1] bitmap:0x55555555_55555555, next->null, prev->[0]
    */
    if (slab64->bitmap != 0b11 || slab64->next == NULL ||
        slab64->prev != NULL) {
        goto failed;
    }
    entry = slab64->next;
    if (entry->bitmap != 0x5555555555555555 || entry->next != NULL ||
        entry->prev != slab64) {
        goto failed;
    }
    debugf("       pass test2\n");

    for (int i = 0; i < 64; i++) {
        addr2[i] = kmalloc(64);
    }
    /* expected:
        [0] bitmap:0bFFFFFFFF_FFFFFFFF, next->[1], prev->null (used: 2->64)
        [1] bitmap:0x55555555_5555555F, next->null, prev->[0] (0b0101 -> 0b111)
    */
    if (slab64->bitmap != 0xFFFFFFFFFFFFFFFF || slab64->next == NULL ||
        slab64->prev != NULL) {
        goto failed;
    }
    entry = slab64->next;
    if (entry->bitmap != 0x555555555555555F || entry->next != NULL ||
        entry->prev != slab64) {
        goto failed;
    }
    debugf("       pass test3\n");

    for (int i = 1; i < 64; i += 2) {
        kmfree(addr[i]);
    }
    for (int i = 0; i < 64; i++) {
        kmfree(addr2[i]);
    }
    /* expected:
        [0] bitmap:0b1, next->null, prev->null
    */
    if (slab64->bitmap != 0b1 || slab64->next != NULL || slab64->prev != NULL) {
        goto failed;
    }
    debugf("       pass test4\n");
    debugf("[slab] finish slab_tests\n");
    return;

failed:
    entry = slab64;
    while (entry != NULL) {
        debugf("Dump slab64: %x\n", entry);
        debugf("             bitmap %x\n", entry->bitmap);
        debugf("             prev %x\n", entry->prev);
        debugf("             next %x\n", entry->next);
        entry = entry->next;
    }
    panic("slab_tests failed\n");
}
