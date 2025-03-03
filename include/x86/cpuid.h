#pragma once

enum {
    CPUID_EAX,
    CPUID_EBX,
    CPUID_ECX,
    CPUID_EDX,
};

static inline uint32 cpuid(uint32 code, uint8 regs) {
    uint32 eax, ebx, ecx, edx;
    asm volatile("cpuid"
                 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                 : "a"(code));
    switch (regs) {
        case CPUID_EAX:
            return eax;
        case CPUID_EBX:
            return ebx;
        case CPUID_ECX:
            return ecx;
        case CPUID_EDX:
            return edx;
        default:
            kprintf("cpuid: invalid register %d\n", regs);
            return 0;
    }
}

#define CPUID_0001_ECX cpuid(1, CPUID_ECX)
#define CPUID_0001_ECX_SSE3 (1 << 0)
#define CPUID_0001_ECX_PCLMULQDQ (1 << 1)
#define CPUID_0001_ECX_DTES64 (1 << 2)
#define CPUID_0001_ECX_MONITOR (1 << 3)
#define CPUID_0001_ECX_DS_CPL (1 << 4)
#define CPUID_0001_ECX_VMX (1 << 5)
#define CPUID_0001_ECX_SMX (1 << 6)
#define CPUID_0001_ECX_EST (1 << 7)
#define CPUID_0001_ECX_TM2 (1 << 8)
#define CPUID_0001_ECX_SSSE3 (1 << 9)
#define CPUID_0001_ECX_CNXT_ID (1 << 10)
#define CPUID_0001_ECX_FMA (1 << 12)
#define CPUID_0001_ECX_CMPXCHG16B (1 << 13)
#define CPUID_0001_ECX_XTPR (1 << 14)
#define CPUID_0001_ECX_PDCM (1 << 15)
#define CPUID_0001_ECX_PCID (1 << 17)
#define CPUID_0001_ECX_DCA (1 << 18)
#define CPUID_0001_ECX_SSE4_1 (1 << 19)
#define CPUID_0001_ECX_SSE4_2 (1 << 20)
#define CPUID_0001_ECX_X2APIC (1 << 21)
#define CPUID_0001_ECX_MOVBE (1 << 22)
#define CPUID_0001_ECX_POPCNT (1 << 23)
#define CPUID_0001_ECX_TSC_DEADLINE (1 << 24)
#define CPUID_0001_ECX_AES (1 << 25)
#define CPUID_0001_ECX_XSAVE (1 << 26)
#define CPUID_0001_ECX_OSXSAVE (1 << 27)
#define CPUID_0001_ECX_AVX (1 << 28)
#define CPUID_0001_ECX_F16C (1 << 29)
#define CPUID_0001_ECX_RDRAND (1 << 30)

#define CPUID_0001_EDX cpuid(1, CPUID_EDX)
#define CPUID_0001_EDX_FPU (1 << 0)
#define CPUID_0001_EDX_VME (1 << 1)
#define CPUID_0001_EDX_DE (1 << 2)
#define CPUID_0001_EDX_PSE (1 << 3)
#define CPUID_0001_EDX_TSC (1 << 4)
#define CPUID_0001_EDX_MSR (1 << 5)
#define CPUID_0001_EDX_PAE (1 << 6)
#define CPUID_0001_EDX_MCE (1 << 7)
#define CPUID_0001_EDX_CX8 (1 << 8)
#define CPUID_0001_EDX_APIC (1 << 9)
#define CPUID_0001_EDX_SEP (1 << 11)
#define CPUID_0001_EDX_MTRR (1 << 12)
#define CPUID_0001_EDX_PGE (1 << 13)
#define CPUID_0001_EDX_MCA (1 << 14)
#define CPUID_0001_EDX_CMOV (1 << 15)
#define CPUID_0001_EDX_PAT (1 << 16)
#define CPUID_0001_EDX_PSE36 (1 << 17)
#define CPUID_0001_EDX_PSN (1 << 18)
#define CPUID_0001_EDX_CFLSH (1 << 19)
#define CPUID_0001_EDX_DS (1 << 21)
#define CPUID_0001_EDX_ACPI (1 << 22)
#define CPUID_0001_EDX_MMX (1 << 23)
#define CPUID_0001_EDX_FXSR (1 << 24)
#define CPUID_0001_EDX_SSE (1 << 25)
#define CPUID_0001_EDX_SSE2 (1 << 26)
#define CPUID_0001_EDX_SS (1 << 27)
#define CPUID_0001_EDX_HTT (1 << 28)
#define CPUID_0001_EDX_TM (1 << 29)
#define CPUID_0001_EDX_PBE (1 << 31)

#define CPUID_8000_0001_EDX cpuid(0x80000001, CPUID_EDX)
#define CPUID_8000_0001_EDX_SYSCALL (1 << 11)
#define CPUID_8000_0001_EDX_PDPE1GB (1 << 26)

// physical/linear address size
#define CPUID_8000_0008_EAX cpuid(0x80000008, CPUID_EAX)
