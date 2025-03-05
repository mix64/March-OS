#include <fs/fat16.h>
#include <fs/inode.h>
#include <lib/string.h>
#include <mm.h>

extern FAT16 fat16;
void fat16_read_cluster(uint16 cluster, void *buf);

int fat16_readi(inode_t *ino, void *dst, uint64 offset, uint64 size) {
    if (ino->id > 0xFFFF) {
        return -1;
    }

    if (offset + size > ino->size) {
        return -1;
    }

    uint16 cluster = (uint16)ino->id;
    uint16 cluster_offset = offset / fat16.cluster_size;
    uint16 cluster_offset_remain = offset % fat16.cluster_size;
    uint16 cluster_offset_end = (offset + size) / fat16.cluster_size;
    uint16 cluster_offset_end_remain = (offset + size) % fat16.cluster_size;

    void *buf = (void *)kmalloc(fat16.cluster_size);

    for (int i = cluster_offset; i <= cluster_offset_end; i++) {
        fat16_read_cluster(cluster + i, buf);
        if (i == cluster_offset && i == cluster_offset_end) {
            // under one cluster
            memcpy(dst, buf + cluster_offset_remain, size);
        } else if (i == cluster_offset) {
            // first cluster
            memcpy(dst, buf + cluster_offset_remain,
                   fat16.cluster_size - cluster_offset_remain);
        } else if (i == cluster_offset_end) {
            // last cluster
            memcpy(dst + (i - cluster_offset) * fat16.cluster_size, buf,
                   cluster_offset_end_remain);
        } else {
            // middle cluster
            memcpy(dst + (i - cluster_offset) * fat16.cluster_size, buf,
                   fat16.cluster_size);
        }
    }

    kmfree(buf);
    return 0;
}