#include <elf.h>
#include <fs/inode.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
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
    Elf64_Ehdr ehdr;
    vfs_read(inode, &ehdr, 0, sizeof(Elf64_Ehdr));

    for (int i = 0, off = ehdr.e_phoff; i < ehdr.e_phnum;
         i++, off += ehdr.e_phentsize) {
        Elf64_Phdr phdr;
        vfs_read(inode, &phdr, off, sizeof(Elf64_Phdr));

        if (phdr.p_type != ELF_PROG_LOAD) {
            continue;
        }
        if (phdr.p_filesz > phdr.p_memsz) {
            goto out;
        }
        if (phdr.p_vaddr < USER_ADDR_START) {
            goto out;
        }
        vfs_read(inode, (void *)phdr.p_vaddr, phdr.p_offset, phdr.p_filesz);
    }

out:
    ifree(inode);
    return 0;
}
