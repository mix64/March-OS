#pragma once

#include <types.h>

enum inode_type {
    INODE_T_FILE,
    INODE_T_DIR,
    INODE_T_DEV,
};

typedef struct inode {
    uint64 id;    // inode number
    uint64 size;  // file size
    uint16 refcnt;
    enum inode_type type;  // file type
} inode_t;

void iinit();
inode_t *ialloc(uint64 id);
void ifree(inode_t *ip);
