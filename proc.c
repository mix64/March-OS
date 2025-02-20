#include <kernel.h>
#include <lib/string.h>
#include <list.h>
#include <mm.h>
#include <proc.h>
#include <trap.h>
#include <x86/asm.h>

list_t proc_list;
uint64 nextpid;

#define KSTACK_SIZE KiB(1)

extern void context_switch(context_t **old, context_t *new);
extern void _sysret();

char initcode[] = {
    // 40000000 <start>:
    0x6a, 0x00,                               // push $0x0
    0x68, 0x19, 0x00, 0x00, 0x40,             // push $0x40000019 # ($init)
    0x6a, 0x00,                               // push $0x0
    0xb8, 0x0B, 0x00, 0x00, 0x00,             // mov $0x0B, %eax # (SYS_execv)
    0xcd, 0x40,                               // int $0x40
                                              // 40000010 <exit>:
    0xb8, 0x01, 0x00, 0x00, 0x00,             // mov $0x01, %eax
    0xcd, 0x40,                               // int $0x40
    0xeb, 0xf7,                               // jmp exit
                                              // 40000019 <init>:
    0x73, 0x68, 0x65, 0x6c, 0x6c, 0x00, 0x00  // "shell"
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