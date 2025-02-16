#include <boot/info.h>
#include <fs.h>
#include <kernel.h>
#include <mm.h>
#include <pci.h>
#include <system.h>
#include <x86.h>
#include <x86/apic.h>
#include <x86/asm.h>

System SYSTEM;

void kernel_main() {
    kprintf("Hello, Kernel!\n");
    mm_init();
    idt_init();
    apic_init();
    pci_scan();
    fs_init();
    sti();
    while (1);
}

void clear_screen() {
    char *screen = (char *)SYSTEM.screen.base;
    for (uint64 i = 0; i < SYSTEM.screen.size; i += 4) {
        screen[i] = 0xFF;
        screen[i + 1] = 0xFF;
        screen[i + 2] = 0xFF;
        screen[i + 3] = 0x00;
    }
}

void kernel_stub(BootInfo *bi) {
    SYSTEM.memtotal = bi->memtotal;
    SYSTEM.screen.base = bi->screen.base;
    SYSTEM.screen.size = bi->screen.size;
    SYSTEM.screen.hr = bi->screen.hr;
    SYSTEM.screen.vr = bi->screen.vr;
    // clear_screen();
    init_serial();
    kernel_main();
}
