#pragma once
#include <types.h>

#define SLAB_MIN_SIZE 64ULL

enum SLAB_IDX {
    SLAB_64 = 0,
    SLAB_128,
    SLAB_256,
    SLAB_512,
    SLAB_1024,
    SLAB_2048,
    SLAB_4096,
    SLAB_NUM
};

typedef struct SLAB {
    uintptr address;
    uint64 bitmap;
    struct SLAB *next;
} SLAB;

void *kmalloc(uint64 size);
void kmfree(void *addr);
void slab_init();