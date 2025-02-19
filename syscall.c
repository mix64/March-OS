#include <kernel.h>
#include <proc.h>
#include <x86.h>

#define IA32_SYSENTER_CS 0x174
#define IA32_SYSENTER_ESP 0x175
#define IA32_SYSENTER_EIP 0x176

extern void _sysenter();
extern void _sysret();

void syscall_init() {
    debugf("[sys] start setup\n");
    write_msr(IA32_SYSENTER_CS, 0x08);
    write_msr(IA32_SYSENTER_EIP, (uint64)_sysenter);

    debugf("[sys] IA32_SYSENTER_CS: 0x%x\n", read_msr(IA32_SYSENTER_CS));
    debugf("[sys] IA32_SYSENTER_EIP: 0x%x\n", read_msr(IA32_SYSENTER_EIP));
}

void set_sysenter_stack(void *kstack) {
    write_msr(IA32_SYSENTER_ESP, (uint64)kstack);
    debugf("[sys] IA32_SYSENTER_ESP: 0x%x\n", read_msr(IA32_SYSENTER_ESP));
}

void syscall() {
    kprintf("syscall\n");
    while (1);
}