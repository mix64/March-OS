#include <boot/info.h>
#include <serial.h>
#include <system.h>

System SYSTEM;

void clear_screen() {
    char *screen = (char *)SYSTEM.screen.base;
    for (uint64 i = 0; i < SYSTEM.screen.size; i += 4) {
        screen[i] = 0xFF;
        screen[i + 1] = 0xFF;
        screen[i + 2] = 0xFF;
        screen[i + 3] = 0x00;
    }
}

int kernel_main(BootInfo *bi) {
    SYSTEM.memtotal = bi->memtotal;
    SYSTEM.screen.base = bi->screen.base;
    SYSTEM.screen.size = bi->screen.size;
    SYSTEM.screen.hr = bi->screen.hr;
    SYSTEM.screen.vr = bi->screen.vr;
    clear_screen();
    init_serial();
    kprintf("Hello, World!\n");
    while (1);
}