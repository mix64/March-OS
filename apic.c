#include <apic.h>
#include <mm.h>
#include <serial.h>
#include <x86/asm.h>

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

#define APIC_ID_OFFSET 0x20  /* Local APIC ID Register */
#define APIC_EOI_OFFSET 0xB0 /* EOI Register */
#define APIC_SVR_OFFSET 0xF0 /* Spurious Interrupt Vector Register */
#define APIC_SVR_ENABLE (1 << 8)

static uintptr lapic;

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
    lapic = eax & 0xfffff0000;

    debugf("[apic] IA32_APIC_BASE_MSR: %x %x\n", edx, eax);
    debugf("[apic] APIC at: %x\n", lapic);
    if ((eax & IA32_APIC_BASE_MSR_ENABLE) == 0) {
        eax |= IA32_APIC_BASE_MSR_ENABLE;
        wrmsr(IA32_APIC_BASE_MSR, eax, edx);
    }
}

uint32 write_apic(uint32 idx, uint32 value) {
    debugf("[apic] write %x to %x\n", value, lapic + idx);
    mmio_write32((void *)(lapic + idx), value);
    volatile uint32 _ =
        mmio_read32((void *)(lapic + APIC_ID_OFFSET));  // wait for write
    return _;
}

uint32 read_apic(uint32 idx) {
    uint32 data = mmio_read32((void *)(lapic + idx));
    debugf("[apic] read %x from %x\n", data, lapic + idx);
    return data;
}

/* 12.4.3 Enabling or Disabling the Local APIC */
void apic_init() {
    if (!check_apic()) {
        panic("apic: not supported\n");
    }
    if (!check_x2apic()) {
        debugf("[apic] x2apic not supported\n");
    }

    disable_pic();
    enable_apic();
    write_apic(APIC_SVR_OFFSET,
               read_apic(APIC_SVR_OFFSET) | APIC_SVR_ENABLE | 0xFF);
}

void apic_eoi() { write_apic(APIC_EOI_OFFSET, 0); }