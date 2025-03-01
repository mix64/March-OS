#include <kernel.h>
#include <proc.h>
#include <syscall.h>
#include <x86.h>

// sysenter, sysexit
#define IA32_SYSENTER_CS 0x174
#define IA32_SYSENTER_ESP 0x175
#define IA32_SYSENTER_EIP 0x176

// syscall, sysret
#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_FMASK 0xC0000084

#define IA32_EFER 0xC0000080
#define IA32_EFER_SCE 1  // System Call Extensions

extern void _syscall();

// TODO: support syscall,sysret
void syscall_init() {
    debugf("[sys] start setup\n");
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
    write_msr(IA32_FMASK, 0);
    debugf("[sys] IA32_STAR: %x\n", read_msr(IA32_STAR));
    debugf("[sys] IA32_LSTAR: %x\n", read_msr(IA32_LSTAR));
    debugf("[sys] IA32_FMASK: %x\n", read_msr(IA32_FMASK));
    debugf("[sys] setup done\n");
}

void set_sysenter_stack(void *kstack) {
    write_msr(IA32_SYSENTER_ESP, (uint64)kstack);
    debugf("[sys] IA32_SYSENTER_ESP: %x\n", read_msr(IA32_SYSENTER_ESP));
}

void syscall(uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5,
             uint64 arg6) {
    uint64 num;
    __asm__ volatile("movq %%rax, %0" : "=r"(num));

    kprintf("#SYSCALL %d\n", num);
    kprintf("args: %x %x %x %x %x %x\n", arg1, arg2, arg3, arg4, arg5, arg6);
    switch (num) {
        case SYS_spawn:
            kprintf("SYS_spawn\n");
            break;
        case SYS_exit:
            kprintf("SYS_exit\n");
            break;
        default:
            kprintf("unknown syscall %d\n", num);
            break;
    }
    while (1);
}