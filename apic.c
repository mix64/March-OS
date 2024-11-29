#include <asm.h>
#include <serial.h>
#include <x86.h>

/*
 SDM Vol.3B
 24.27 ADVANCED PROGRAMMABLE INTERRUPT CONTROLLER (APIC)

 https://wiki.osdev.org/APIC
*/

#define CPUID_FEAT_RDX_APIC (1 << 9)
#define CPUID_FEAT_RCX_X2APIC (1 << 21)

/* Figure 12-5. IA32_APIC_BASE MSR */
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP (1 << 8)
#define IA32_APIC_BASE_MSR_ENABLE (1 << 11)

#define IA32_APIC_SVR_OFFSET 0xF0 /* Spurious Interrupt Vector Register */
#define IA32_APIC_SVR_ENABLE (1 << 8)

static uint32 APIC_BASE_ADDR;

int check_apic() {
    uint64 rax, rbx, rcx, rdx;
    rax = 1;
    asm volatile("cpuid"
                 : "=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                 : "a"(rax));
    return (rdx & CPUID_FEAT_RDX_APIC);
}

int check_x2apic() {
    uint64 rax, rbx, rcx, rdx;
    rax = 1;
    asm volatile("cpuid"
                 : "=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                 : "a"(rax));
    return (rcx & CPUID_FEAT_RCX_X2APIC);
}

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_DATA (PIC1 + 1)
#define PIC2_DATA (PIC2 + 1)
void disable_pic() {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

/* Set the physical address for local APIC registers */
void enable_apic() {
    uint32 eax, edx = 0;
    rdmsr(IA32_APIC_BASE_MSR, &eax, &edx);
    APIC_BASE_ADDR = eax & 0xfffff0000;

    if (eax & IA32_APIC_BASE_MSR_ENABLE) {
        debugf("apic: already enabled\n");
        return;
    }

    eax = (eax & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    wrmsr(IA32_APIC_BASE_MSR, eax, edx);
}

void write_apic(uint32 reg, uint32 value) {
    volatile uint32 *addr = (uint32 *)(APIC_BASE_ADDR + reg);
    debugf("apic: write %x to %x\n", value, addr);
    *addr = value;
}

uint32 read_apic(uint32 reg) {
    volatile uint32 *addr = (uint32 *)(APIC_BASE_ADDR + reg);
    return *addr;
}

/* 12.4.3 Enabling or Disabling the Local APIC */
void apic_init() {
    if (!check_apic()) {
        panic("apic: not supported\n");
    }
    if (!check_x2apic()) {
        debugf("apic: x2apic not supported\n");
    }

    disable_pic();
    enable_apic();
    uint32 svr = read_apic(IA32_APIC_SVR_OFFSET);
    debugf("apic: svr = %x\n", svr);
    write_apic(IA32_APIC_SVR_OFFSET,
               read_apic(IA32_APIC_SVR_OFFSET) | IA32_APIC_SVR_ENABLE | 0xFF);
    debugf("apic: svr = %x\n", read_apic(IA32_APIC_SVR_OFFSET));
}
