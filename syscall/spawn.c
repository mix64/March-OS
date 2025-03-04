#include <elf.h>
#include <fs/fat16.h>
#include <kernel.h>
#include <lib/string.h>

uint16 get_inode_from_path(char *path);

int sys_spawn(char *path) {
    if (path == NULL) {
        return -1;
    }

    uint16 inode = get_inode_from_path(path);
    if (inode == 0) {
        return -1;
    }

    // Load the file into memory

    return 0;
}

uint16 get_inode_from_path(char *path) {
    char filename[12] = {0};  // FAT16 filename (8.3 format)
    uint16 dinode = 0;        // Start searching from the root directory
    uint16 inode = 0;
    int i = 1;  // Start after initial '/', assuming absolute path
    int j = 0;

    if (path[0] != '/') {
        return 0;  //  Only absolute paths supported
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

        inode = fat16_find_cluster(filename, dinode);
        debugf("sys_spawn: found %s (%d) in %d\n", filename, inode, dinode);

        if (inode == 0) {
            // File/Directory not found
            return 0;
        }

        if (path[i] == '/') {
            // If it's a directory, update dinode and continue
            dinode = inode;
            i++;  // Skip '/'
            if (path[i] == '\0') {
                // path ends with '/'
                return 0;
            }
        } else if (path[i] != '\0') {
            return 0;  // Invalid characters after filename
        } else {
            return inode;  // Found the file
        }

        // Reset filename for next part of the path
        memset(filename, 0, 12);
    }
    return 0;
}
