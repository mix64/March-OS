#include <fs/fat16.h>
#include <fs/inode.h>
#include <vfs.h>

int vfs_read(inode_t *ip, char *dst, uint64 offset, uint64 size) {
    // return fat16_readi(ip, dst, offset, size);
    return -1;
}

inode_t *vfs_namei(char *path) { return fat16_namei(path); }

extern void fat16_init();
void vfs_init() { fat16_init(); }