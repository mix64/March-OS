#pragma once

#include <fs/inode.h>
#include <types.h>

#define SECTOR_SIZE 512

int vfs_read(inode_t *ip, char *dst, uint64 offset, uint64 size);
int vfs_write(inode_t *ip, char *src, uint64 offset, uint64 size);
inode_t *vfs_namei(char *path);
inode_t *vfs_getrootdir();