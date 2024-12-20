#pragma once
#include <types.h>

// Global Descriptor Table
enum {
    GDT_SEG_NULL,
    GDT_SEG_KCODE,
    GDT_SEG_KDATA,
    GDT_SEG_UCODE,
    GDT_SEG_UDATA,
    GDT_SEG_TSS,

    GDT_SEG_COUNT,
};

#define GDT_KCODE (GDT_SEG_KCODE << 3)
#define GDT_KDATA (GDT_SEG_KDATA << 3)
#define GDT_UCODE (GDT_SEG_UCODE << 3)
#define GDT_UDATA (GDT_SEG_UDATA << 3)
#define GDT_TSS (GDT_SEG_TSS << 3)

struct seg32desc {
    uint64 lim_15_0 : 16;
    uint64 base_15_0 : 16;
    uint64 base_23_16 : 8;
    uint64 type : 4;
    uint64 s : 1;
    uint64 dpl : 2;
    uint64 p : 1;
    uint64 lim_19_16 : 4;
    uint64 flags : 4;
    uint64 base_31_24;
};

struct seg64desc {
    uint64 lim_15_0 : 16;
    uint64 base_15_0 : 16;
    uint64 base_23_16 : 8;
    uint64 type : 4;
    uint64 s : 1;
    uint64 dpl : 2;
    uint64 p : 1;
    uint64 lim_19_16 : 4;
    uint64 flags : 4;
    uint64 base_31_24 : 8;
    uint64 base_63_32 : 32;
    uint64 reserved : 32;
};

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
