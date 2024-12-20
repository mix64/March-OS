#pragma once
#include <types.h>

struct gatedesc {
    uint64 off_15_0 : 16;    // low 16 bits of offset in segment
    uint64 sel : 16;         // code segment selector
    uint64 IST : 3;          // Interrupt Stack Table
    uint64 reserved_2 : 5;   // reserved
    uint64 type : 4;         // type(STS_{IG64,TG64})
    uint64 s : 1;            // must be 0 (system)
    uint64 dpl : 2;          // Descriptor Privilege Level
    uint64 p : 1;            // Present
    uint64 off_31_16 : 16;   // middle 16 bits of offset in segment
    uint64 off_63_32 : 32;   // high 32 bits of offset in segment
    uint64 reserved_1 : 32;  // reserved
};

#define STS_IG64 0xE  // 64-bit Interrupt Gate
#define STS_TG64 0xF  // 64-bit Trap Gate

// x86.c
void idt_init();
