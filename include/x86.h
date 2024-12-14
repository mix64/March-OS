#pragma once
#include <types.h>

struct gatedesc {
    uint16 off_15_0;        // low 16 bits of offset in segment
    uint16 sel;             // code segment selector
    uint16 IST : 3;         // Interrupt Stack Table
    uint16 reserved_2 : 5;  // reserved
    uint16 type : 4;        // type(STS_{IG64,TG64})
    uint16 s : 1;           // must be 0 (system)
    uint16 dpl : 2;         // Descriptor Privilege Level
    uint16 p : 1;           // Present
    uint16 off_31_16;       // middle 16 bits of offset in segment
    uint32 off_63_32;       // high 32 bits of offset in segment
    uint32 reserved_1;      // reserved
};

#define STS_IG64 0xE  // 64-bit Interrupt Gate
#define STS_TG64 0xF  // 64-bit Trap Gate

void idt_init();
void set_idt_entry(struct gatedesc *idt, uint16 cs, uint64 offset,
                   uint8 is_trap, uint8 dpl);
void gdt_init();
