#include <kernel.h>

int sys_spawn(char *filename) {
    kprintf("sys_spawn: %s\n", filename);
    return 0;
}