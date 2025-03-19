#include <file.h>
#include <fs/devfs.h>
#include <fs/inode.h>

int stdio_read(file_t *file, void *buf, uint64 size) { return 0; }

static struct file_operations stdio_fops = {
    .open = NULL,
    .close = NULL,
    .read = stdio_read,
    .write = NULL,
    .seek = NULL,
};

static file_t stdio = {
    .fops = &stdio_fops,
};

file_t *devfs_stdio() { return &stdio; }