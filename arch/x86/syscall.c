#include <kernel.h>
#include <x86/desc.h>
#include <x86/msr.h>

extern void _syscall();

void syscall_init() {
    write_msr(IA32_EFER, read_msr(IA32_EFER) | IA32_EFER_SCE);
    debugf("[sys] IA32_EFER: %x\n", read_msr(IA32_EFER));

    // for sysenter, sysexit
    write_msr(IA32_SYSENTER_CS, 0x08);
    write_msr(IA32_SYSENTER_EIP, (uint64)_syscall);
    debugf("[sys] IA32_SYSENTER_CS: %x\n", read_msr(IA32_SYSENTER_CS));
    debugf("[sys] IA32_SYSENTER_EIP: %x\n", read_msr(IA32_SYSENTER_EIP));

    // for syscall, sysret
    write_msr(IA32_STAR, ((uint64)GDT_KCODE << 32) | ((uint64)GDT_UNULL << 48));
    write_msr(IA32_LSTAR, (uint64)_syscall);
    write_msr(IA32_SFMASK, 0);
    debugf("[sys] IA32_STAR: %x\n", read_msr(IA32_STAR));
    debugf("[sys] IA32_LSTAR: %x\n", read_msr(IA32_LSTAR));
    debugf("[sys] IA32_FMASK: %x\n", read_msr(IA32_SFMASK));
}

void set_sysenter_stack(void *kstack) {
    write_msr(IA32_SYSENTER_ESP, (uint64)kstack);
    debugf("[sys] IA32_SYSENTER_ESP: %x\n", read_msr(IA32_SYSENTER_ESP));
}
