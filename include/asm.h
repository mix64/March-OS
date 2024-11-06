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
