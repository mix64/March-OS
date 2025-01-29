#include <mm.h>
#include <serial.h>
#include <types.h>

#define __SLAB_DEBUG__
struct slab64_info {
    uint64 bitmap;
    struct slab64_info *prev;
    struct slab64_info *next;
};

static struct slab64_info *slab64;

void dump_slab64();

void slab_init() {
    slab64 = (struct slab64_info *)kalloc();
    slab64->bitmap = 0b1;
    slab64->prev = NULL;
    slab64->next = NULL;

#ifdef __SLAB_DEBUG__
    void *addr[128];
    for (int i = 0; i < 64; i++) {
        addr[i] = kmalloc(64);
    }
    debugf("[mm] kmalloc(64) test\n");
    dump_slab64();
    debugf("[mm] kmfree(64) test\n");
    for (int i = 0; i < 64; i += 2) {
        kmfree(addr[i]);
    }
    dump_slab64();
    debugf("kalloc() %x\n", kalloc());
    debugf("[mm] kmalloc(64) test2\n");
    for (int i = 0; i < 128; i++) {
        addr[i] = kmalloc(64);
    }
    dump_slab64();
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

void dump_slab64() {
    struct slab64_info *entry = slab64;
    while (entry != NULL) {
        debugf("Dump slab64: %x\n", entry);
        debugf("             bitmap %x\n", entry->bitmap);
        debugf("             prev %x\n", entry->prev);
        debugf("             next %x\n", entry->next);
        entry = entry->next;
    }
}