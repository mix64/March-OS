#include <apic.h>
#include <kernel.h>
#include <trap.h>
#include <x86/asm.h>
#include <x86/cpuid.h>
#include <x86/msr.h>

/*
 SDM Vol.3B
 24.27 ADVANCED PROGRAMMABLE INTERRUPT CONTROLLER (APIC)

 https://wiki.osdev.org/APIC
*/

#define APIC_ID 0x20  /* Local APIC ID Register */
#define APIC_VER 0x30 /* Local APIC Version Register */
#define APIC_TPR 0x80 /* Task Priority Register */
#define APIC_EOI 0xB0 /* EOI Register */
#define APIC_SVR 0xF0 /* Spurious Interrupt Vector Register */
#define APIC_SVR_ENABLE (1 << 8)
#define APIC_ESR 0x280 /* Error Status Register */

/* 12.6.1 Interrupt Command Register (ICR) */
#define APIC_ICR_LOW 0x300              /* Interrupt Command Register [31:0] */
#define APIC_ICR_INIT (0b101 << 8)      /* INIT/RESET */
#define APIC_ICR_STARTUP (0b110 << 8)   /* Start-up IPI */
#define APIC_ICR_DELIV_STATUS (1 << 12) /* Delivery status */
#define APIC_ICR_ASSERT (1 << 14)       /* Assert interrupt (vs deassert) */
#define APIC_ICR_LEVEL (1 << 15)        /* Level triggered */
#define APIC_ICR_BROADCAST (0b10 << 18) /* Broadcast (including self) */
#define APIC_ICR_HIGH 0x310             /* Interrupt Command Register [63:32] */

/* 12.5.4 APIC Timer */
#define APIC_LVT_TIMER 0x320              /* LVT Timer */
#define APIC_LVT_TIMER_PERIODIC (1 << 17) /* Periodic Timer Mode */
#define APIC_LVT_PMC 0x340                /* Performance Monitor Counter */
#define APIC_LVT_LINT0 0x350              /* Local Vector Table 1 (LINT0) */
#define APIC_LVT_LINT1 0x360              /* Local Vector Table 2 (LINT1) */
#define APIC_LVT_ERROR 0x370              /* Local Vector Table 3 (ERROR) */
#define APIC_LVT_MASKED (1 << 16)         /* Masked */
#define APIC_TIMER_ICR 0x380 /* Initial Count Register (for Timer) */
#define APIC_TIMER_ICR_VALUE 10000000
#define APIC_TIMER_DCR 0x3E0 /* Divide Configuration Register (for Timer) */
#define APIC_TIMER_DCR_DIV1 0xB

/* IO APIC */
#define IOAPIC_REG_INDEX 0x00
#define IOAPIC_REG_DATA 0x10
#define IOAPIC_REDTBL_START 0x10

APIC apic;

int check_apic() { return (CPUID_0001_EDX & CPUID_0001_EDX_APIC); }

int check_x2apic() { return (CPUID_0001_ECX & CPUID_0001_ECX_X2APIC); }

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
    apic.lapic_addr = eax & 0xfffff0000;

    debugf("[apic] IA32_APIC_BASE_MSR: %x %x\n", edx, eax);
    debugf("[apic] APIC at: %x\n", apic.lapic_addr);
    if ((eax & IA32_APIC_BASE_MSR_ENABLE) == 0) {
        eax |= IA32_APIC_BASE_MSR_ENABLE;
        wrmsr(IA32_APIC_BASE_MSR, eax, edx);
    }
}

uint32 write_apic(uint32 idx, uint32 value) {
    mmio_write32((void *)(apic.lapic_addr + idx), value);
    volatile uint32 _ =
        mmio_read32((void *)(apic.lapic_addr + APIC_ID));  // wait for write
    return _;
}

uint32 read_apic(uint32 idx) {
    uint32 data = mmio_read32((void *)(apic.lapic_addr + idx));
    return data;
}

uint32 read_ioapic(uint8 reg) {
    mmio_write32((void *)(apic.ioapic_addr + IOAPIC_REG_INDEX), reg);
    return mmio_read32((void *)(apic.ioapic_addr + IOAPIC_REG_DATA));
}

void write_ioapic(uint8 reg, uint32 value) {
    mmio_write32((void *)(apic.ioapic_addr + IOAPIC_REG_INDEX), reg);
    mmio_write32((void *)(apic.ioapic_addr + IOAPIC_REG_DATA), value);
}

void ioapic_init() {
    debugf("[apic] IO APIC at: %x\n", apic.ioapic_addr);
    // Setup Keyboard (IRQ1)
    // Bits 0-7: Interrupt vector.
    write_ioapic(IOAPIC_REDTBL_START + IRQ_KBD * 2, T_IRQ0 + IRQ_KBD);
    // Bits 56-63: Destination field.
    write_ioapic(IOAPIC_REDTBL_START + IRQ_KBD * 2 + 1, 0);
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
    write_apic(APIC_SVR, APIC_SVR_ENABLE | (T_IRQ0 + IRQ_SPURIOUS));

    // Setup timer
    write_apic(APIC_TIMER_DCR, APIC_TIMER_DCR_DIV1);
    write_apic(APIC_LVT_TIMER, APIC_LVT_TIMER_PERIODIC | (T_IRQ0 + IRQ_TIMER));
    write_apic(APIC_TIMER_ICR, APIC_TIMER_ICR_VALUE);

    // Setup LVT
    write_apic(APIC_LVT_LINT0, APIC_LVT_MASKED);
    write_apic(APIC_LVT_LINT1, APIC_LVT_MASKED);
    if (((read_apic(APIC_VER) >> 16) & 0xFF) >= 4) {
        write_apic(APIC_LVT_PMC, APIC_LVT_MASKED);
    }
    write_apic(APIC_LVT_ERROR, T_IRQ0 + IRQ_ERROR);
    write_apic(APIC_ESR, 0);
    write_apic(APIC_ESR, 0);
    write_apic(APIC_EOI, 0);

    // Send INIT IPI
    write_apic(APIC_ICR_HIGH, 0);
    write_apic(APIC_ICR_LOW,
               APIC_ICR_BROADCAST | APIC_ICR_LEVEL | APIC_ICR_INIT);
    while (read_apic(APIC_ICR_LOW) & APIC_ICR_DELIV_STATUS);
    write_apic(APIC_TPR, 0);

    // Setup IOAPIC
    ioapic_init();
}

void apic_eoi() { write_apic(APIC_EOI, 0); }
