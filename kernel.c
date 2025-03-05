#include <boot/info.h>
#include <kernel.h>
#include <mm.h>
#include <pci.h>
#include <proc.h>
#include <system.h>
#include <x86/apic.h>
#include <x86/asm.h>

System SYSTEM;

extern void syscall_init();
extern void cpu_init();
extern void serial_init();
extern void idt_init();
extern void vfs_init();

void kernel_main() {
    kprintf("Hello, Kernel!\n");
    cpu_init();
    mm_init();
    idt_init();
    apic_init();
    pci_scan();
    vfs_init();
    syscall_init();
    uinit();
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
    serial_init();
    kernel_main();
}
