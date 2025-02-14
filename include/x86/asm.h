#pragma once
#include <types.h>

static inline void outb(uint16 port, uint8 v) {
    asm volatile("outb %0,%1" : : "a"(v), "dN"(port));
}

static inline void outw(uint16 port, uint16 v) {
    asm volatile("outw %0,%1" : : "a"(v), "dN"(port));
}

static inline void outl(uint16 port, uint32 v) {
    asm volatile("outl %0, %1" : : "a"(v), "dN"(port));
}

static inline uint8 inb(uint16 port) {
    uint8 v;
    asm volatile("inb %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline uint16 inw(uint16 port) {
    uint16 v;
    asm volatile("inw %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline uint32 inl(uint16 port) {
    uint32 v;
    asm volatile("inl %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void cli() { asm volatile("cli"); }

static inline void sti() { asm volatile("sti"); }

static inline uintptr lcr2() {
    uintptr addr;
    asm volatile("movq %%cr2,%0" : "=r"(addr));
    return addr;
}

static inline uintptr lcr3() {
    uintptr addr;
    asm volatile("movq %%cr3,%0" : "=r"(addr));
    return addr;
}

static inline void scr3(void *addr) {
    asm volatile("movq %0,%%cr3" : : "r"(addr));
}

static inline uint64 cpuid(uint64 code) {
    uint64 rax, rbx, rcx, rdx;
    asm volatile("cpuid"
                 : "=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                 : "a"(code));
    return rax;
}

#define MAX_PADDR_BITS (cpuid(0x80000008) & 0xFF)

static inline void stosb(void *addr, uint64 data, uint64 cnt) {
    asm volatile("cld; rep stosb"
                 : "=D"(addr), "=c"(cnt)
                 : "0"(addr), "1"(cnt), "a"(data)
                 : "memory", "cc");
}

static inline void movsb(void *dst, void *src, uint64 cnt) {
    asm volatile("cld; rep movsb"
                 : "=D"(dst), "=S"(src), "=c"(cnt)
                 : "0"(dst), "1"(src), "2"(cnt)
                 : "memory", "cc");
}

static inline void rdmsr(uint32 msr, uint32 *lo, uint32 *hi) {
    asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void wrmsr(uint32 msr, uint32 lo, uint32 hi) {
    asm volatile("wrmsr" ::"a"(lo), "d"(hi), "c"(msr));
}

static inline uint32 mmio_read32(void *addr) {
    uint32 data;
    asm volatile("movl (%1), %k0" : "=r"(data) : "r"(addr) : "memory");
    return data;
}

static inline void mmio_write32(void *addr, uint32 data) {
    asm volatile("movl %k0, (%1)" : : "r"(data), "r"(addr) : "memory");
}