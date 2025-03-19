#pragma once

#define DEVFS_STDIO 0

void devfs_init();
struct file *devfs_stdio();
