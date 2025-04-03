#pragma once

#include <types.h>

// ACPI Specification
// 5. ACPI Software Programming Model
// https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html

// 5.2.5.3. Root System Description Pointer (RSDP) Structure
typedef struct __attribute__((packed)) {
    char signature[8];
    uint8 checksum;
    char OEMID[6];
    uint8 revision;
    uint32 rsdt_address;
} ACPI_RSDP;

typedef struct __attribute__((packed)) {
    char Signature[8];
    uint8 Checksum;
    char OEMID[6];
    uint8 Revision;
    uint32 RsdtAddress;
    uint32 Length;
    uint64 XsdtAddress;
    uint8 ExtendedChecksum;
    uint8 Reserved[3];
} ACPI_XSDP;

// 5.2.6. System Description Table Header
typedef struct __attribute__((packed)) {
    char Signature[4];
    uint32 Length;
    uint8 Revision;
    uint8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32 OEMRevision;
    uint32 CreatorID;
    uint32 CreatorRevision;
} ACPI_RSDT_HEADER;

typedef struct __attribute__((packed)) {
    char Signature[4];
    uint32 Length;
    uint8 Revision;
    uint8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32 OEMRevision;
    uint32 CreatorID;
    uint32 CreatorRevision;
} ACPI_XSDT_HEADER;

// 5.2.12. Multiple APIC Description Table (MADT)
typedef struct __attribute__((packed)) {
    char Signature[4];  // "ACPI"
    uint32 Length;
    uint8 Revision;
    uint8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32 OEMRevision;
    uint32 CreatorID;
    uint32 CreatorRevision;
    uint32 LocalAPICAddress;
    uint32 Flags;  // 1 = Dual 8259 Legacy PICs Installed
} ACPI_MADT;

typedef struct __attribute__((packed)) {
    uint8 entry_type;
    uint8 length;
} ACPI_MADT_HEAD;

#define ACPI_MADT_T_LAPIC 0
#define ACPI_MADT_T_IOAPIC 1
#define ACPI_MADT_T_INTERRUPT_OVERRIDE 2
#define ACPI_MADT_T_NMI 3
#define ACPI_MADT_T_LAPIC_NMI 4
#define ACPI_MADT_T_LAPIC_ADDRESS_OVERRIDE 5
#define ACPI_MADT_T_LOCAL_X2APIC 9

// 0: Processor Local APIC
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint8 acpi_processor_id;
    uint8 apic_id;
    uint32 flags;  // (bit 0 = Processor Enabled, bit 1 = Online Capable)
} ACPI_MADT_LAPIC;

// 1: I/O APIC
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint8 ioapic_id;
    uint8 reserved;  // 0
    uint32 ioapic_address;
    uint32 global_system_interrupt_base;
} ACPI_MADT_IOAPIC;

// 2: I/O APIC Interrupt Source Override
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint8 bus_source;
    uint8 irq_source;
    uint32 global_system_interrupt;
    uint16 flags;
} ACPI_MADT_INTERRUPT_OVERRIDE;

// 3: I/O APIC Non-maskable interrupt source
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint8 nmi_source;
    uint8 reserved;
    uint16 flags;  // 0
    uint32 global_system_interrupt;
} ACPI_MADT_NMI;

// 4: Local APIC Non-maskable interrupts
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint8 acpi_processor_id;
    uint16 flags;
    uint8 lint;
} ACPI_MADT_LAPIC_NMI;

// 5: Local APIC Address Override
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint16 reserved;
    uint64 local_apic_address;
} ACPI_MADT_LAPIC_ADDRESS_OVERRIDE;

// 9: Processor Local x2APIC
typedef struct __attribute__((packed)) {
    ACPI_MADT_HEAD head;
    uint16 reserved;
    uint32 local_x2apic_id;
    uint32 flags;
    uint32 acpi_id;
} ACPI_MADT_LOCAL_X2APIC;