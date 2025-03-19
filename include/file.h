#pragma once

#include <types.h>

#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2

typedef struct file {
    struct inode *inode;
    uint64 offset;
    struct file_operations *fops;
} file_t;

struct file_operations {
    int (*open)(file_t *file, const char *path, int flags);
    int (*close)(file_t *file);
    int (*read)(file_t *file, void *buf, uint64 size);
    int (*write)(file_t *file, void *buf, uint64 size);
    int (*seek)(file_t *file, uint64 offset);
};
