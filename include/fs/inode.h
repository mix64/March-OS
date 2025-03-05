#pragma once

#include <types.h>

typedef struct inode {
    uint64 id;    // inode number
    uint64 size;  // file size
    uint16 refcnt;
} inode_t;

inode_t *ialloc(uint64 id);
void ifree(inode_t *ip);
