#include <x86/asm.h>
#include <x86/msr.h>

uint64 read_msr(uint32 msr) {
    uint32 lo, hi;
    rdmsr(msr, &lo, &hi);
    return lo | ((uint64)hi << 32);
}

void write_msr(uint32 msr, uint64 val) { wrmsr(msr, val, val >> 32); }