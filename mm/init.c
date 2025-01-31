#include <mm.h>

extern void pm_init();
extern void vm_init();
extern void slab_init();

void mm_init() {
    pm_init();
    vm_init();
    slab_init();
}