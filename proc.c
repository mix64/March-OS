#include <kernel.h>
#include <lib/string.h>
#include <list.h>
#include <mm.h>
#include <proc.h>
#include <trap.h>
#include <x86/asm.h>

list_t proc_list;
uint64 nextpid;

extern void trapret();
extern void swtch(context_t **old, context_t *new);

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
    proc_t *init = palloc();
    init->stat = RUN;
    switch_uvm(init->upml4);
    memcpy((void *)USER_ADDR_START, initcode, sizeof(initcode));
}

proc_t *palloc() {
    proc_t *p = (proc_t *)kmalloc(sizeof(proc_t));
    p->stat = SET;
    p->pid = nextpid++;
    p->kstack = kmalloc(KiB(1));
    uint64 sp = (uint64)p->kstack + KiB(1);
    sp -= sizeof(struct trapframe);
    p->tf = (struct trapframe *)sp;
    sp -= sizeof(context_t);
    p->context = (context_t *)sp;
    p->wchan = NULL;
    p->upml4 = (uintptr)pmalloc(PM_4K) | PG_P | PG_RW | PG_US;

    p->context->rip = (uint64)trapret;

    list_push(&proc_list, p);
    return p;
}

void pfree(proc_t *p) {
    list_remove(&proc_list, p);
    kmfree(p->kstack);
    pmfree((void *)p->upml4, PM_4K);
    kmfree(p);
}