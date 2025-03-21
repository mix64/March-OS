#include <elf.h>
#include <fs/inode.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <proc.h>
#include <vfs.h>

int sys_spawn(char *path, char *argv[]) {
    if (path == NULL) {
        return -1;
    }

    inode_t *inode = vfs_namei(path);
    if (inode == NULL) {
        return -1;
    }
    proc_t *new_proc = palloc();
    if (new_proc == NULL) {
        ifree(inode);
        return -1;
    }
    switch_uvm(new_proc);

    // Load the file into memory
    Elf64_Ehdr ehdr;
    if (vfs_read(inode, &ehdr, 0, sizeof(Elf64_Ehdr)) == -1) {
        goto bad;
    }
    debugf(" entry %x\n", ehdr.e_entry);

    for (int i = 0, off = ehdr.e_phoff; i < ehdr.e_phnum;
         i++, off += ehdr.e_phentsize) {
        Elf64_Phdr phdr;
        if (vfs_read(inode, &phdr, off, sizeof(Elf64_Phdr)) == -1) {
            goto bad;
        }

        if (phdr.p_type != ELF_PROG_LOAD) {
            continue;
        }
        if (phdr.p_filesz > phdr.p_memsz) {
            goto bad;
        }
        if (phdr.p_vaddr < USER_ADDR_START) {
            goto bad;
        }
        if (vfs_read(inode, (void *)phdr.p_vaddr, phdr.p_offset,
                     phdr.p_filesz) == -1) {
            goto bad;
        }
        debugf(" mapped %x-%x\n", phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz);
    }

    goto out;
bad:
    pfree(new_proc);

out:
    // restore the current process
    switch_uvm(curproc);
    ifree(inode);
    return 0;
}
