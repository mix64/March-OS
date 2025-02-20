#pragma once
#include <types.h>

// Global Descriptor Table
enum {
    GDT_SEG_NULL,
    GDT_SEG_KCODE,
    GDT_SEG_KDATA,
    GDT_SEG_UNULL,
    GDT_SEG_UDATA,
    GDT_SEG_UCODE,
    GDT_SEG_TSS,

    GDT_SEG_COUNT,
};

#define GDT_KCODE (GDT_SEG_KCODE << 3)
#define GDT_KDATA (GDT_SEG_KDATA << 3)
#define GDT_UNULL (GDT_SEG_UNULL << 3)
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
    uint64 base_31_24 : 8;
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

struct taskstate {
    uint64 reserved_0 : 32;

    // The Stack Pointers used to load the stack when a privilege level change
    // occurs from a lower privilege level to a higher one.
    uint64 rsp0_lo : 32;
    uint64 rsp0_hi : 32;
    uint64 rsp1_lo : 32;
    uint64 rsp1_hi : 32;
    uint64 rsp2_lo : 32;
    uint64 rsp2_hi : 32;

    uint64 reserved_1 : 32;
    uint64 reserved_2 : 32;

    // Interrupt Stack Table.
    // The Stack Pointers used to load the stack when an entry in the IDT has an
    // IST value other than 0.
    // In our case, we will not use this feature.
    uint64 ist1_lo : 32;
    uint64 ist1_hi : 32;
    uint64 ist2_lo : 32;
    uint64 ist2_hi : 32;
    uint64 ist3_lo : 32;
    uint64 ist3_hi : 32;
    uint64 ist4_lo : 32;
    uint64 ist4_hi : 32;
    uint64 ist5_lo : 32;
    uint64 ist5_hi : 32;
    uint64 ist6_lo : 32;
    uint64 ist6_hi : 32;
    uint64 ist7_lo : 32;
    uint64 ist7_hi : 32;

    uint64 reserved_3 : 32;
    uint64 reserved_4 : 32;
    uint64 reserved_5 : 16;

    // I/O Map Base Address Field.
    // 16-bit offset from the base of the TSS to the I/O Permission Bit Map.
    uint64 iomb : 16;
};

// x86.c
void idt_init();
uint64 read_msr(uint32 msr);
void write_msr(uint32 msr, uint64 val);