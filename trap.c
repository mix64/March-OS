#include <kernel.h>
#include <trap.h>
#include <x86/apic.h>

static uint64 ticks;  // 100Hz Timer Counter

void dump_tf(struct trapframe *tf);

void trap(struct trapframe *tf) {
    switch (tf->trapno) {
        case T_IRQ0 + IRQ_TIMER:
            ticks++;
            apic_eoi();
            if (ticks % 100 == 0) {
                kprintf("ticks = %d\n", ticks);
            }
            break;
        default:
            kprintf("unknown interrupt\n");
            dump_tf(tf);
            break;
    }
    return;
}

void dump_tf(struct trapframe *tf) {
    kprintf("trapframe:\n");
    kprintf("    trapno = %x\n", tf->trapno);
    kprintf("    r15 = %x\n", tf->r15);
    kprintf("    r14 = %x\n", tf->r14);
    kprintf("    r13 = %x\n", tf->r13);
    kprintf("    r12 = %x\n", tf->r12);
    kprintf("    r11 = %x\n", tf->r11);
    kprintf("    r10 = %x\n", tf->r10);
    kprintf("     r9 = %x\n", tf->r9);
    kprintf("     r8 = %x\n", tf->r8);
    kprintf("    rdi = %x\n", tf->rdi);
    kprintf("    rsi = %x\n", tf->rsi);
    kprintf("    rbp = %x\n", tf->rbp);
    kprintf("    rbx = %x\n", tf->rbx);
    kprintf("    rdx = %x\n", tf->rdx);
    kprintf("    rcx = %x\n", tf->rcx);
    kprintf("    rax = %x\n", tf->rax);
    kprintf("    err = %x\n", tf->err);
    kprintf("    rip = %x\n", tf->rip);
    kprintf("    rsp = %x\n", tf->rsp);
    kprintf("    cs  = %x\n", tf->cs);
    kprintf("    ss  = %x\n", tf->ss);
    kprintf("    rflags = %x\n", tf->rflags);
}