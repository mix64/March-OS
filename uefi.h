#pragma once
#include "types.h"

typedef struct {
    void *base;
    uint64 size;
    uint32 hr;
    uint32 vr;
} FrameBuffer;

typedef struct {
    uint64 memtotal;
    FrameBuffer screen;
} KernelParams;
