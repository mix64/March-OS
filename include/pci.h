#pragma once

#include <types.h>

enum pci_header_type {
    PCI_HEADER_TYPE_GENERAL = 0,
    PCI_HEADER_TYPE_BRIDGE = 1,
    PCI_HEADER_TYPE_CARDBUS = 2,
};

enum pci_class_code {
    PCI_CLASS_CODE_UNCLASSIFIED = 0x00,
    PCI_CLASS_CODE_STORAGE = 0x01,
    PCI_CLASS_CODE_NETWORK = 0x02,
    PCI_CLASS_CODE_DISPLAY = 0x03,
    PCI_CLASS_CODE_MULTIMEDIA = 0x04,
    PCI_CLASS_CODE_MEMORY = 0x05,
    PCI_CLASS_CODE_BRIDGE = 0x06,
    PCI_CLASS_CODE_SIMPLE_COMM = 0x07,
    PCI_CLASS_CODE_BASE_SYSTEM_PERIPHERAL = 0x08,
    PCI_CLASS_CODE_INPUT_DEVICE = 0x09,
    PCI_CLASS_CODE_DOCKING_STATION = 0x0A,
    PCI_CLASS_CODE_PROCESSOR = 0x0B,
    PCI_CLASS_CODE_SERIAL_BUS = 0x0C,
    PCI_CLASS_CODE_WIRELESS = 0x0D,
    PCI_CLASS_CODE_INTELLIGENT_IO = 0x0E,
    PCI_CLASS_CODE_SATELLITE_COMM = 0x0F,
    PCI_CLASS_CODE_ENCRYPTION = 0x10,
    PCI_CLASS_CODE_SIGNAL = 0x11,
    PCI_CLASS_CODE_PROCESSING_ACCELERATOR = 0x12,
    PCI_CLASS_CODE_NON_ESSENTIAL = 0x13,
    PCI_CLASS_CODE_COPROCESSOR = 0x40,
    PCI_CLASS_CODE_UNASSIGNED = 0xFF,
};

struct pci_device_header {
    uint8 bus;
    uint8 slot;
    uint8 func;

    // Common Header Fields
    uint16 vendor_id;
    uint16 device_id;
    uint16 command;
    uint16 status;
    uint8 rev_id;
    uint8 prog_if;
    uint8 subclass_id;
    uint8 class_id;
    uint8 cache_line_size;
    uint8 latency_timer;
    uint8 header_type;
    uint8 bist;
};

struct pci_device {
    struct pci_device_header header;

    // General PCI Device Fields
    uint32 bars[6];
    uint32 cardbus_cis_pointer;
    uint16 subsystem_vendor_id;
    uint16 subsystem_id;
    uint32 expansion_rom_base;
    uint8 cap_pointer;
    uint8 interrupt_line;
    uint8 interrupt_pin;
    uint8 min_grant;
    uint8 max_latency;
};

void pci_scan();