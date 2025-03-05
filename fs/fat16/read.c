#include <fs/fat16.h>
#include <fs/inode.h>

int fat16_readi(inode_t *ino, char *dst, uint64 offset, uint64 size) {
    if (ino->id > 0xFFFF) {
        return -1;
    }
    return 0;
}