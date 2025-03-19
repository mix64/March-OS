#include <fs/devfs.h>
#include <fs/fat16.h>
#include <fs/inode.h>
#include <vfs.h>

int vfs_read(inode_t *ip, void *dst, uint64 offset, uint64 size) {
    return fat16_readi(ip, dst, offset, size);
}

inode_t *vfs_namei(char *path) { return fat16_namei(path); }

void vfs_init() {
    devfs_init();
    fat16_init();
}