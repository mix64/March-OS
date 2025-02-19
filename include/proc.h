#pragma once
#include <types.h>

enum procstat { UNUSED, SET, READY, RUN, SLEEP, ZOMB, STOP };

// callee-saved registers (System V AMD64 ABI)
typedef struct context {
    uint64 r15;
    uint64 r14;
    uint64 r13;
    uint64 r12;
    uint64 rbp;
    uint64 rbx;
    uint64 rcx;  // ring3 stack
    uint64 rdx;  // ring3 rip
    uint64 rip;
} context_t;

typedef struct proc {
    enum procstat stat;
    uint64 pid;
    void *kstack;  // kernel stack for this process (IA32_SYSENTER_ESP)
    context_t *context;
    void *wchan;  // waiting channel
    pte_t upml4;  // page directory (PML[2] for user process)
} proc_t;

// proc.c
proc_t *palloc();
void pfree(proc_t *p);
void uinit();

// syscall.c
void set_sysenter_stack(void *kstack);
