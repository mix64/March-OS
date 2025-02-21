#include <kernel.h>
#include <x86/cpuid.h>

void cpu_init() {
    // check syscall/sysret
    if (CPUID_8000_0001_EDX & CPUID_8000_0001_EDX_SYSCALL) {
        kprintf("[cpu] SYSCALL/SYSRET is supported\n");
    } else {
        panic("[cpu] SYSCALL/SYSRET is not supported\n");
    }

    // check 1GB page
    if (CPUID_8000_0001_EDX & CPUID_8000_0001_EDX_PDPE1GB) {
        kprintf("[cpu] 1GB page is supported\n");
    } else {
        kprintf("[cpu] 1GB page is not supported\n");
    }

    // check processor brand
    uint64 processor_brand[12];
    for (int i = 0; i < 12; i++) {
        processor_brand[i] = cpuid(0x80000002 + i / 4, CPUID_EAX + (i % 4));
        if (processor_brand[i] == 0) {
            break;
        }
    }
    kprintf("[cpu] processor brand: %s\n", (char*)processor_brand);

    // check processor features
    kprintf("[cpu] features: ");
    {
        uint32 edx = CPUID_0001_EDX;
        if (edx & CPUID_0001_EDX_FPU) kprintf("FPU ");
        if (edx & CPUID_0001_EDX_VME) kprintf("VME ");
        if (edx & CPUID_0001_EDX_DE) kprintf("DE ");
        if (edx & CPUID_0001_EDX_PSE) kprintf("PSE ");
        if (edx & CPUID_0001_EDX_TSC) kprintf("TSC ");
        if (edx & CPUID_0001_EDX_MSR) kprintf("MSR ");
        if (edx & CPUID_0001_EDX_PAE) kprintf("PAE ");
        if (edx & CPUID_0001_EDX_MCE) kprintf("MCE ");
        if (edx & CPUID_0001_EDX_CX8) kprintf("CX8 ");
        if (edx & CPUID_0001_EDX_APIC) kprintf("APIC ");
        if (edx & CPUID_0001_EDX_SEP) kprintf("SEP ");
        if (edx & CPUID_0001_EDX_MTRR) kprintf("MTRR ");
        if (edx & CPUID_0001_EDX_PGE) kprintf("PGE ");
        if (edx & CPUID_0001_EDX_MCA) kprintf("MCA ");
        if (edx & CPUID_0001_EDX_CMOV) kprintf("CMOV ");
        if (edx & CPUID_0001_EDX_PAT) kprintf("PAT ");
        if (edx & CPUID_0001_EDX_PSE36) kprintf("PSE-36 ");
        if (edx & CPUID_0001_EDX_PSN) kprintf("PSN ");
        if (edx & CPUID_0001_EDX_CFLSH) kprintf("CFLSH ");
        if (edx & CPUID_0001_EDX_DS) kprintf("DS ");
        if (edx & CPUID_0001_EDX_ACPI) kprintf("ACPI ");
        if (edx & CPUID_0001_EDX_MMX) kprintf("MMX ");
        if (edx & CPUID_0001_EDX_FXSR) kprintf("FXSR ");
        if (edx & CPUID_0001_EDX_SSE) kprintf("SSE ");
        if (edx & CPUID_0001_EDX_SSE2) kprintf("SSE2 ");
        if (edx & CPUID_0001_EDX_SS) kprintf("SS ");
        if (edx & CPUID_0001_EDX_HTT) kprintf("HTT ");
        if (edx & CPUID_0001_EDX_TM) kprintf("TM ");
        if (edx & CPUID_0001_EDX_PBE) kprintf("PBE ");

        uint32 ecx = CPUID_0001_ECX;
        if (ecx & CPUID_0001_ECX_SSE3) kprintf("SSE3 ");
        if (ecx & CPUID_0001_ECX_PCLMULQDQ) kprintf("PCLMULQDQ ");
        if (ecx & CPUID_0001_ECX_DTES64) kprintf("DTES64 ");
        if (ecx & CPUID_0001_ECX_MONITOR) kprintf("MONITOR ");
        if (ecx & CPUID_0001_ECX_DS_CPL) kprintf("DS_CPL ");
        if (ecx & CPUID_0001_ECX_VMX) kprintf("VMX ");
        if (ecx & CPUID_0001_ECX_SMX) kprintf("SMX ");
        if (ecx & CPUID_0001_ECX_EST) kprintf("EST ");
        if (ecx & CPUID_0001_ECX_TM2) kprintf("TM2 ");
        if (ecx & CPUID_0001_ECX_SSSE3) kprintf("SSSE3 ");
        if (ecx & CPUID_0001_ECX_CNXT_ID) kprintf("CNXT_ID ");
        if (ecx & CPUID_0001_ECX_FMA) kprintf("FMA ");
        if (ecx & CPUID_0001_ECX_CMPXCHG16B) kprintf("CMPXCHG16B ");
        if (ecx & CPUID_0001_ECX_XTPR) kprintf("XTPR ");
        if (ecx & CPUID_0001_ECX_PDCM) kprintf("PDCM ");
        if (ecx & CPUID_0001_ECX_PCID) kprintf("PCID ");
        if (ecx & CPUID_0001_ECX_DCA) kprintf("DCA ");
        if (ecx & CPUID_0001_ECX_SSE4_1) kprintf("SSE4.1 ");
        if (ecx & CPUID_0001_ECX_SSE4_2) kprintf("SSE4.2 ");
        if (ecx & CPUID_0001_ECX_X2APIC) kprintf("X2APIC ");
        if (ecx & CPUID_0001_ECX_MOVBE) kprintf("MOVBE ");
        if (ecx & CPUID_0001_ECX_POPCNT) kprintf("POPCNT ");
        if (ecx & CPUID_0001_ECX_TSC_DEADLINE) kprintf("TSC_DEADLINE ");
        if (ecx & CPUID_0001_ECX_AES) kprintf("AES ");
        if (ecx & CPUID_0001_ECX_XSAVE) kprintf("XSAVE ");
        if (ecx & CPUID_0001_ECX_OSXSAVE) kprintf("OSXSAVE ");
        if (ecx & CPUID_0001_ECX_AVX) kprintf("AVX ");
        if (ecx & CPUID_0001_ECX_F16C) kprintf("F16C ");
        if (ecx & CPUID_0001_ECX_RDRAND) kprintf("RDRAND ");
        kprintf("\n");
    }

    //
}