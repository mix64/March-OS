#include <serial.h>
#include <trap.h>

void dump(struct trapframe *tf);

void trap(struct trapframe *tf) {
    dump(tf);
    return;
}

void dump(struct trapframe *tf) {
    kprintf("tf->r15 = %x\n", tf->r15);
    kprintf("tf->r14 = %x\n", tf->r14);
    kprintf("tf->r13 = %x\n", tf->r13);
    kprintf("tf->r12 = %x\n", tf->r12);
    kprintf("tf->r11 = %x\n", tf->r11);
    kprintf("tf->r10 = %x\n", tf->r10);
    kprintf("tf->r9 = %x\n", tf->r9);
    kprintf("tf->r8 = %x\n", tf->r8);
    kprintf("tf->rdi = %x\n", tf->rdi);
    kprintf("tf->rsi = %x\n", tf->rsi);
    kprintf("tf->rbp = %x\n", tf->rbp);
    kprintf("tf->rbx = %x\n", tf->rbx);
    kprintf("tf->rdx = %x\n", tf->rdx);
    kprintf("tf->rcx = %x\n", tf->rcx);
    kprintf("tf->rax = %x\n", tf->rax);
    kprintf("tf->trapno = %x\n", tf->trapno);
    kprintf("tf->err = %x\n", tf->err);
    kprintf("tf->rip = %x\n", tf->rip);
    kprintf("tf->cs = %x\n", tf->cs);
    kprintf("tf->rflags = %x\n", tf->rflags);
    kprintf("tf->rsp = %x\n", tf->rsp);
    kprintf("tf->ss = %x\n", tf->ss);
}