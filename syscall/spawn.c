#include <elf.h>
#include <fs/inode.h>
#include <kernel.h>
#include <vfs.h>

int sys_spawn(char *path, char *argv[]) {
    if (path == NULL) {
        return -1;
    }

    inode_t *inode = vfs_namei(path);
    if (inode == NULL) {
        return -1;
    }

    // Load the file into memory

    ifree(inode);
    return 0;
}
