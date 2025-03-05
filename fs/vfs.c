#include <fs/fat16.h>
#include <fs/inode.h>
#include <vfs.h>

#define __FAT16__

int vfs_read(inode_t *ip, char *dst, uint64 offset, uint64 size) {
#ifdef __FAT16__
    // return fat16_read(ip, dst, offset, size);
#endif
    return -1;
}

extern void fat16_init();
void vfs_init() {
#ifdef __FAT16__
    fat16_init();
#endif
}