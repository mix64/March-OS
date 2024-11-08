#pragma once
#include <types.h>

static inline void outb(uint16 port, uint8 v) {
    asm volatile("outb %0,%1" : : "a"(v), "dN"(port));
}

static inline uint8 inb(uint16 port) {
    uint8 v;
    asm volatile("inb %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void cli() { asm volatile("cli"); }

static inline void sti() { asm volatile("sti"); }

static inline uint64 lcr3() {
    uint64 val;
    asm volatile("movq %%cr3,%0" : "=r"(val));
    return val;
}