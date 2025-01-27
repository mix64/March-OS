#include <mm.h>
#include <serial.h>
#include <types.h>

struct slab64_info {
    uintptr addr;
    uint64 bitmap;
    struct slab64_info *next;
};

static struct slab64_info *slab64;

void dump_slab64();

void slab_init() {
    slab64 = (struct slab64_info *)kalloc();
    slab64->addr = (uintptr)slab64;
    slab64->bitmap = 0b1;
    slab64->next = NULL;

#ifdef __DEBUG__
    void *addr[128];
    for (int i = 0; i < 128; i++) {
        addr[i] = kmalloc(64);
    }
    debugf("kmalloc(64) test\n");
    dump_slab64();
    debugf("kmfree(64) test\n");
    for (int i = 0; i < 128; i++) {
        kmfree(addr[i]);
    }
    dump_slab64();
#endif
}

void *kmalloc(uint64 size) {
    if (size <= 64) {
        if (slab64 == NULL || slab64->addr == 0) {
            panic("kmalloc: slab64");
        }
        for (int i = 1; i < 64; i++) {
            if ((slab64->bitmap & (1 << i)) == 0) {
                slab64->bitmap |= (1 << i);
                return (void *)(slab64->addr + i * 64);
            }
        }
        struct slab64_info *next = slab64;
        slab64 = (struct slab64_info *)kalloc();
        slab64->addr = (uintptr)slab64;
        slab64->bitmap = 0b11;
        slab64->next = next;

        return (void *)(slab64->addr + 64);
    }
    // TODO: Implement kmalloc for other size
    return NULL;
}

void kmfree(void *addr_p) {
    if (addr_p == NULL) {
        panic("kmfree: addr == NULL\n");
    }
    if (slab64->addr == 0) {
        panic("kmfree: slab64->addr == 0\n");
    }

    uintptr addr = (uintptr)addr_p;
    uintptr base = ROUNDDOWN(addr, PAGE_SIZE);
    struct slab64_info *entry = slab64;

    while (entry != NULL) {
        if (entry->addr == base) {
            int idx = (addr - base) / 64;
            if (addr % 64 != 0 || idx < 0 || idx > 63) {
                panic("kmfree: invalid addr %x\n", addr);
            }
            entry->bitmap &= ~(1 << idx);
            // TODO: free physical page if all slots are free
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
        debugf("             addr %x\n", entry->addr);
        debugf("             bitmap %x\n", entry->bitmap);
        debugf("             next %x\n", entry->next);
        entry = entry->next;
    }
}