#include <pci.h>
#include <serial.h>
#include <types.h>
#include <x86/asm.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_ENABLE (1 << 31)

#define PCI_CONFIG_DATA 0xCFC

#define PCI_DEV_MAX 32
static struct pci_device pci_devices[PCI_DEV_MAX];

uint32 pci_config_read(uint8 bus, uint8 slot, uint8 func, uint8 offset) {
    uint32 _bus = (uint32)bus;
    uint32 _slot = (uint32)slot;
    uint32 _func = (uint32)func;

    uint32 address = PCI_CONFIG_ENABLE | (_bus << 16) | (_slot << 11) |
                     (_func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16 pci_config_read16(uint8 bus, uint8 slot, uint8 func, uint8 offset) {
    uint32 data = pci_config_read(bus, slot, func, offset);
    return (uint16)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8 pci_config_read8(uint8 bus, uint8 slot, uint8 func, uint8 offset) {
    uint32 data = pci_config_read(bus, slot, func, offset);
    return (uint8)((data >> ((offset & 3) * 8)) & 0xFF);
}

void pci_read_header(uint8 bus, uint8 slot, uint8 func,
                     struct pci_device_header *header) {
    header->bus = bus;
    header->slot = slot;
    header->func = func;

    header->vendor_id = pci_config_read16(bus, slot, func, 0);
    header->device_id = pci_config_read16(bus, slot, func, 2);
    header->command = pci_config_read16(bus, slot, func, 4);
    header->status = pci_config_read16(bus, slot, func, 6);
    header->rev_id = pci_config_read8(bus, slot, func, 8);
    header->prog_if = pci_config_read8(bus, slot, func, 9);
    header->subclass_id = pci_config_read8(bus, slot, func, 10);
    header->class_id = pci_config_read8(bus, slot, func, 11);
    header->cache_line_size = pci_config_read8(bus, slot, func, 12);
    header->latency_timer = pci_config_read8(bus, slot, func, 13);
    header->header_type = pci_config_read8(bus, slot, func, 14);
    header->bist = pci_config_read8(bus, slot, func, 15);

    kprintf("[PCI] Found Device:\n");
    kprintf("        Vendor: %x\n", header->vendor_id);
    kprintf("        Device: %x\n", header->device_id);
    kprintf("        Type: %x\n", header->header_type);
    debugf("        Status: %x\n", header->status);
    debugf("        Command: %x\n", header->command);
    debugf("        Class: %x\n", header->class_id);
    debugf("        Subclass: %x\n", header->subclass_id);
    debugf("        Prog IF: %x\n", header->prog_if);
    debugf("        Rev ID: %x\n", header->rev_id);
    debugf("        BIST: %x\n", header->bist);
    debugf("        Latency Timer: %x\n", header->latency_timer);
    debugf("        Cache Line Size: %x\n", header->cache_line_size);
}

// General PCI Device Fields
void pci_read_table(struct pci_device *pcidev) {
    uint8 bus = pcidev->header.bus;
    uint8 slot = pcidev->header.slot;
    uint8 func = pcidev->header.func;

    pcidev->bars[0] = pci_config_read(bus, slot, func, 0x10);
    pcidev->bars[1] = pci_config_read(bus, slot, func, 0x14);
    pcidev->bars[2] = pci_config_read(bus, slot, func, 0x18);
    pcidev->bars[3] = pci_config_read(bus, slot, func, 0x1C);
    pcidev->bars[4] = pci_config_read(bus, slot, func, 0x20);
    pcidev->bars[5] = pci_config_read(bus, slot, func, 0x24);
    pcidev->cardbus_cis_pointer = pci_config_read(bus, slot, func, 0x28);
    pcidev->subsystem_vendor_id = pci_config_read16(bus, slot, func, 0x2C);
    pcidev->subsystem_id = pci_config_read16(bus, slot, func, 0x2E);
    pcidev->expansion_rom_base = pci_config_read(bus, slot, func, 0x30);
    pcidev->cap_pointer = pci_config_read8(bus, slot, func, 0x34);
    pcidev->interrupt_line = pci_config_read8(bus, slot, func, 0x3C);
    pcidev->interrupt_pin = pci_config_read8(bus, slot, func, 0x3D);
    pcidev->min_grant = pci_config_read8(bus, slot, func, 0x3E);
    pcidev->max_latency = pci_config_read8(bus, slot, func, 0x3F);

    debugf("        BAR0: %x\n", pcidev->bars[0]);
    debugf("        BAR1: %x\n", pcidev->bars[1]);
    debugf("        BAR2: %x\n", pcidev->bars[2]);
    debugf("        BAR3: %x\n", pcidev->bars[3]);
    debugf("        BAR4: %x\n", pcidev->bars[4]);
    debugf("        BAR5: %x\n", pcidev->bars[5]);
    debugf("        Cardbus CIS Pointer: %x\n", pcidev->cardbus_cis_pointer);
    debugf("        Subsystem Vendor ID: %x\n", pcidev->subsystem_vendor_id);
    debugf("        Subsystem ID: %x\n", pcidev->subsystem_id);
    debugf("        Expansion ROM Base: %x\n", pcidev->expansion_rom_base);
    debugf("        Cap Pointer: %x\n", pcidev->cap_pointer);
    debugf("        Interrupt Line: %x\n", pcidev->interrupt_line);
    debugf("        Interrupt Pin: %x\n", pcidev->interrupt_pin);
    debugf("        Min Grant: %x\n", pcidev->min_grant);
    debugf("        Max Latency: %x\n", pcidev->max_latency);
}

void pci_scan() {
    int idx = 0;
    for (uint32 bus = 0; bus < 256; bus++) {
        for (uint32 slot = 0; slot < 32; slot++) {
            if (pci_config_read16(bus, slot, 0, 0) == 0xFFFF) {
                continue;
            }
            pci_read_header(bus, slot, 0, &pci_devices[idx].header);
            pci_read_table(&pci_devices[idx]);

            if (idx < PCI_DEV_MAX) {
                idx++;
            } else {
                panic("PCI Device limit reached\n");
            }
        }
    }
}