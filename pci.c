#include <pci.h>
#include <serial.h>
#include <types.h>
#include <x86/asm.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_ENABLE (1 << 31)

#define PCI_CONFIG_DATA 0xCFC

uint16 pci_config_read16(uint32 bus, uint32 slot, uint32 func, uint32 offset) {
    uint32 address = PCI_CONFIG_ENABLE | (bus << 16) | (slot << 11) |
                     (func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return (uint16)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

void check_device(uint16 bus, uint8 dev) {
    uint16 vendor = pci_config_read16(bus, dev, 0, 0);
    if (vendor == 0xFFFF) {
        return;
    }
    uint16 device = pci_config_read16(bus, dev, 0, 2);
    debugf("[PCI] Found device: %x:%x\n", vendor, device);
}

void pci_scan() {
    for (uint16 bus = 0; bus < 256; bus++) {
        for (uint8 dev = 0; dev < 32; dev++) {
            check_device(bus, dev);
        }
    }
}