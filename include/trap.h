#include <types.h>

// x86 exception and interrupt constants.
#define X86_EX_PF 14

#define T_IRQ0 32
#define IRQ_TIMER 0
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

struct trapframe {
    // registers as pushed by vectors.S
    uint64 r15;
    uint64 r14;
    uint64 r13;
    uint64 r12;
    uint64 r11;
    uint64 r10;
    uint64 r9;
    uint64 r8;
    uint64 rdi;
    uint64 rsi;
    uint64 rbp;
    uint64 rbx;
    uint64 rdx;
    uint64 rcx;
    uint64 rax;
    uint64 trapno;

    // below here defined by x86 hardware
    uint64 err;
    uint64 rip;
    uint64 cs;
    uint64 rflags;

    // below here only when crossing rings, such as from user to kernel
    uint64 rsp;
    uint64 ss;
};

void trap(struct trapframe *tf);