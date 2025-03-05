#include <kernel.h>
#include <lib/string.h>
#include <list.h>
#include <mm.h>
#include <proc.h>
#include <trap.h>
#include <x86/asm.h>
#include <x86/mm.h>

list_t proc_list;
proc_t *curproc;
uint64 nextpid;

#define KSTACK_SIZE KiB(1)

extern void context_switch(context_t **old, context_t *new);
extern void _sysret();

char initcode[] = {
    // 10000000000 <start>:
    0x48, 0xbf, 0x26, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00,                                      // movabs $0x100000026, %rdi
    0x48, 0x31, 0xf6,                          // xor %rsi, %rsi
    0x48, 0xc7, 0xc0, 0xe8, 0x03, 0x00, 0x00,  // mov $0x3e8, %rax
    0x0f, 0x05,                                // syscall
    // 0000010000000016 <exit>:
    0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,  // mov $0x1, %rdi
    0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,  // mov $0x3c, %rax
    0x0f, 0x05,                                // syscall
    // 0000010000000026 <init>:
    0x2f, 0x42, 0x49, 0x4e, 0x2f, 0x53, 0x48, 0x45, 0x4c, 0x4c,
    0x00,  // "/BIN/SHELL\0"
};

void uinit() {
    proc_t dummy;
    context_t dummy_context;
    dummy.context = &dummy_context;

    proc_t *init = palloc();
    init->stat = RUN;
    switch_uvm(init->upml4);
    set_sysenter_stack(init->kstack + KSTACK_SIZE);
    memcpy((void *)USER_ADDR_START, initcode, sizeof(initcode));
    init->context->r15 = USER_ADDR_START;
    init->context->r14 = USER_ADDR_START + KiB(4);
    curproc = init;
    context_switch(&dummy.context, init->context);
}

void switch_proc(proc_t *new, proc_t *old) {
    if (new == NULL || old == NULL) {
        panic("switch_proc: new/old is NULL\n");
    }
    if (old->stat != RUN || new->stat != READY) {
        panic("switch_proc: stat is invalid\n");
    }
    old->stat = READY;
    new->stat = RUN;
    curproc = new;
    switch_uvm(new->upml4);
    set_sysenter_stack(new->kstack + KSTACK_SIZE);
    context_switch(&old->context, new->context);
}

proc_t *palloc() {
    proc_t *p = (proc_t *)kmalloc(sizeof(proc_t));
    p->stat = SET;
    p->pid = nextpid++;
    p->kstack = kmalloc(KSTACK_SIZE);
    uint64 sp = (uint64)p->kstack + KSTACK_SIZE;
    sp -= sizeof(context_t);
    p->context = (context_t *)sp;
    p->context->rip = (uint64)_sysret;
    p->wchan = NULL;
    p->upml4 = (uintptr)pmalloc(PM_4K) | PG_P | PG_RW | PG_US;

    list_push(&proc_list, p);
    return p;
}

void pfree(proc_t *p) {
    list_remove(&proc_list, p);
    kmfree(p->kstack);
    pmfree((void *)p->upml4, PM_4K);
    kmfree(p);
}