#pragma once

enum {
    CPUID_EAX,
    CPUID_EBX,
    CPUID_ECX,
    CPUID_EDX,
};

static inline uint64 cpuid(uint64 code, uint64 regs) {
    uint64 rax, rbx, rcx, rdx;
    asm volatile("cpuid"
                 : "=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                 : "a"(code));
    switch (regs) {
        case CPUID_EAX:
            return rax;
        case CPUID_EBX:
            return rbx;
        case CPUID_ECX:
            return rcx;
        case CPUID_EDX:
            return rdx;
        default:
            kprintf("cpuid: invalid register %d\n", regs);
            return 0;
    }
}

#define CPUID_0001_RCX cpuid(1, CPUID_ECX)
#define CPUID_0001_RCX_X2APIC (1 << 21)
#define CPUID_0001_RDX cpuid(1, CPUID_EDX)
#define CPUID_0001_RDX_APIC (1 << 9)

#define CPUID_8000_0008_EAX cpuid(0x80000008, CPUID_EAX)
