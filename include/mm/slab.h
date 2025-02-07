#pragma once
#include <types.h>

#define SLAB_MIN_SIZE 64ULL

enum SLAB_IDX {
    SLAB_64 = 0,
    SLAB_128,
    SLAB_256,
    SLAB_512,
    SLAB_1K,
    SLAB_2K,
    SLAB_4K,
    SLAB_8K,
    SLAB_16K,
    SLAB_32K,
    SLAB_64K,
    SLAB_128K,
    SLAB_256K,
    SLAB_512K,
    SLAB_NUM
};

typedef struct SLAB {
    uintptr address;
    uint64 bitmap;
    struct SLAB *next;
} SLAB;
