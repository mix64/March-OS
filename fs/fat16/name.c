#include <fs/fat16.h>
#include <fs/inode.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>

extern uint64 fat16_find_cluster(char*, uint16);

inode_t* fat16_namei(char* path) {
    char filename[12] = {0};  // FAT16 filename (8.3 format)
    uint16 dir_cluster = 0;   // Start searching from the root directory
    uint16 cluster = 0;
    uint32 size = 0;
    inode_t* ip = NULL;
    int i = 1;  // Start after initial '/', assuming absolute path
    int j = 0;

    if (path[0] != '/') {
        return NULL;  //  Only absolute paths supported
    }

    while (path[i] != '\0') {
        j = 0;
        // Extract filename
        while (path[i] != '/' && path[i] != '\0') {
            if (j < 11) {  // Check filename length (8 + 3)
                filename[j] = path[i];
                j++;
            }
            i++;
        }
        filename[j] = '\0';  // Null-terminate filename

        uint64 _tmp = fat16_find_cluster(filename, dir_cluster);
        cluster = (uint16)_tmp;
        size = (uint32)(_tmp >> 32);

        if (cluster == 0) {
            // File/Directory not found
            return NULL;
        }

        if (path[i] == '/') {
            // If it's a directory, update dinode and continue
            dir_cluster = cluster;
            i++;  // Skip '/'
            if (path[i] == '\0') {
                // path ends with '/'
                kprintf("sys_spawn: path ends with '/'\n");
                return NULL;
            }
        } else if (path[i] != '\0') {
            return NULL;  // Invalid characters after filename
        } else {
            ip = ialloc(cluster);
            if (ip == NULL) {
                return NULL;
            }
            ip->size = size;
            return ip;  // Found the file
        }

        // Reset filename for next part of the path
        memset(filename, 0, 12);
    }
    return NULL;
}