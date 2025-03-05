#pragma once

#include <types.h>

typedef struct inode {
    uint64 id;    // inode number
    uint64 size;  // file size
} inode_t;
