#pragma once

/* Figure 12-5. IA32_APIC_BASE MSR */
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP (1 << 8)
#define IA32_APIC_BASE_MSR_ENABLE (1 << 11)

// sysenter, sysexit
#define IA32_SYSENTER_CS 0x174
#define IA32_SYSENTER_ESP 0x175
#define IA32_SYSENTER_EIP 0x176

// syscall, sysret
#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_SFMASK 0xC0000084

#define IA32_EFER 0xC0000080
#define IA32_EFER_SCE 1  // System Call Extensions

uint64 read_msr(uint32 msr);
void write_msr(uint32 msr, uint64 val);