#pragma once
#include <types.h>

// System information
typedef struct {
    uint64 memtotal;
    struct FrameBuffer {
        void *base;
        uint64 size;
        uint32 hr;
        uint32 vr;
    } screen;
} System;