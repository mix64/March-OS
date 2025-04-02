#pragma once

#include <types.h>

typedef struct {
    uint64 lapic_id;
    uintptr lapic_addr;
    uint64 ioapic_id;
    uintptr ioapic_addr;
} APIC;

// apic.c
void apic_init();
void apic_eoi();