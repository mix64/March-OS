#include <serial.h>
#include <x86.h>

struct gatedesc idt[256];
static uint16 idtr[5];
extern uint64 vectors[];
extern struct seg32desc gdt[GDT_SEG_COUNT];

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