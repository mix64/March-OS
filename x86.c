#include <mm.h>
#include <serial.h>
#include <x86.h>

struct gatedesc idt[256];
struct taskstate ts;
static uint16 idtr[5];
extern uint64 vectors[];
extern struct seg32desc gdt[];

void idt_entry(struct gatedesc *idt, uint16 cs, uint64 offset, uint8 is_trap,
               uint8 dpl);

void idt_init() {
    debugf("[x86] init Interrupt Descriptor Table\n");
    for (int i = 0; i < 256; i++) {
        idt_entry(&idt[i], GDT_KCODE, vectors[i], 0, 0);
    }
    idtr[0] = sizeof(idt) - 1;
    idtr[1] = (uint64)idt;
    idtr[2] = (uint64)idt >> 16;
    idtr[3] = (uint64)idt >> 32;
    idtr[4] = (uint64)idt >> 48;
    asm volatile("lidt (%0)" ::"r"(idtr));

    debugf("[x86] init Task State Segment\n");
    ts.rsp0_lo = (uint64)kalloc() + PAGE_SIZE;
    // ts.ist1_lo = (uint32)kalloc() + PAGE_SIZE;  // only use IST1
    ts.iomb = (uint16)0xFFFF;
    struct seg64desc *tss = (struct seg64desc *)&gdt[GDT_SEG_TSS];
    tss->lim_15_0 = (uint16)(sizeof(ts) - 1);
    tss->base_15_0 = (uint16)(((uint64)&ts) & 0xFFFF);
    tss->base_23_16 = (uint8)(((uint64)&ts >> 16) & 0xFF);
    tss->type = 0x9;  // 64-bit TSS (Available)
    tss->s = 0;       // must 0 in TSS
    tss->dpl = 0;     // 0 for kernel
    tss->p = 1;       // Present
    tss->lim_19_16 = (uint8)((sizeof(ts) - 1) >> 16);
    tss->flags = 0;
    tss->base_31_24 = (uint8)(((uint64)&ts >> 24) & 0xFF);
    tss->base_63_32 = (uint32)((uint64)&ts >> 32);
    asm volatile("ltr %0" ::"r"((uint16)GDT_TSS));

    debugf("[x86] init IDT/TSS Done.\n");
}

void idt_entry(struct gatedesc *idt, uint16 cs, uint64 offset, uint8 is_trap,
               uint8 dpl) {
    idt->off_63_32 = (uint32)(offset >> 32);
    idt->off_31_16 = (uint16)((offset >> 16) & 0xFFFF);
    idt->off_15_0 = (uint16)(offset) & 0xFFFF;
    idt->sel = cs;
    idt->p = 1;
    idt->dpl = dpl;
    idt->type = (is_trap) ? STS_TG64 : STS_IG64;
    idt->s = 0;
    idt->IST = 0;
    idt->reserved_1 = 0;
    idt->reserved_2 = 0;
}

void set_idt_entry(uint8 idx, uint64 offset) {
    idt_entry(&idt[idx], GDT_KCODE, offset, 0, 0);
}

void restore_idt_entry(uint8 idx) {
    idt_entry(&idt[idx], GDT_KCODE, vectors[idx], 0, 0);
}