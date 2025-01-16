#include <pci.h>
#include <serial.h>
#include <types.h>
#include <x86/asm.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_ENABLE (1 << 31)

#define PCI_CONFIG_DATA 0xCFC

#define PCI_DEV_MAX 32
static struct pci_device pci_devices[PCI_DEV_MAX];

void pci_parse_class(struct pci_device_header *header);
void pci_dump(struct pci_device *pcidev);

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
            pci_parse_class(&pci_devices[idx].header);
            pci_dump(&pci_devices[idx]);

            if (idx < PCI_DEV_MAX) {
                idx++;
            } else {
                panic("PCI Device limit reached\n");
            }
        }
    }
}

void pci_dump(struct pci_device *pcidev) {
    struct pci_device_header *header = &pcidev->header;
    debugf("        Vendor: %x\n", header->vendor_id);
    debugf("        Device: %x\n", header->device_id);
    debugf("        Type: %x\n", header->header_type);
    debugf("        Status: %x\n", header->status);
    debugf("        Command: %x\n", header->command);
    debugf("        Class: %x\n", header->class_id);
    debugf("        Subclass: %x\n", header->subclass_id);
    debugf("        Prog IF: %x\n", header->prog_if);
    debugf("        Rev ID: %x\n", header->rev_id);
    debugf("        BIST: %x\n", header->bist);
    debugf("        Latency Timer: %x\n", header->latency_timer);
    debugf("        Cache Line Size: %x\n", header->cache_line_size);
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

void pci_parse_class(struct pci_device_header *header) {
    uint8 class_id = header->class_id;
    uint8 subclass_id = header->subclass_id;
    uint8 prog_if = header->prog_if;

    kprintf("[PCI] ");
    switch (class_id) {
        case PCI_CLASS_CODE_UNCLASSIFIED:
            kprintf("Unclassified: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Non-VGA-Compatible Device\n");
                    break;
                case 0x1:
                    kprintf("VGA-Compatible Device\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_STORAGE:
            kprintf("Mass Storage Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("SCSI Bus Controller\n");
                    break;
                case 0x1:
                    kprintf("IDE Controller\n");
                    break;
                case 0x2:
                    kprintf("Floppy Disk Controller\n");
                    break;
                case 0x3:
                    kprintf("IPI Bus Controller\n");
                    break;
                case 0x4:
                    kprintf("RAID Controller\n");
                    break;
                case 0x5:
                    kprintf("ATA Controller ");
                    if (prog_if == 0x20) {
                        kprintf("(Single DMA)\n");
                    } else if (prog_if == 0x30) {
                        kprintf("(Chained DMA)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x6:
                    kprintf("SATA Controller ");
                    if (prog_if == 0) {
                        kprintf("(Vendor Specific Interface)\n");
                    } else if (prog_if == 1) {
                        kprintf("(AHCI 1.0)\n");
                    } else if (prog_if == 2) {
                        kprintf("(Serial Storage Wire Interface)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x7:
                    kprintf("Serial Attached SCSI Controller ");
                    if (prog_if == 0) {
                        kprintf("(SAS)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Serial Storage Bus)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x8:
                    kprintf("Non-Volatile Memory Controller ");
                    if (prog_if == 0) {
                        kprintf("(NVMHCI)\n");
                    } else if (prog_if == 1) {
                        kprintf("(NVM Express)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x80:
                    kprintf("Other Mass Storage Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_NETWORK:
            kprintf("Network Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Ethernet Controller\n");
                    break;
                case 0x1:
                    kprintf("Token Ring Controller\n");
                    break;
                case 0x2:
                    kprintf("FDDI Controller\n");
                    break;
                case 0x3:
                    kprintf("ATM Controller\n");
                    break;
                case 0x4:
                    kprintf("ISDN Controller\n");
                    break;
                case 0x5:
                    kprintf("WorldFip Controller\n");
                    break;
                case 0x6:
                    kprintf("PICMG 2.14 Multi Computing\n");
                    break;
                case 0x7:
                    kprintf("InfiniBand Controller\n");
                    break;
                case 0x8:
                    kprintf("Fabric Controller\n");
                    break;
                case 0x80:
                    kprintf("Other Network Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_DISPLAY:
            kprintf("Display Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("VGA-Compatible Controller ");
                    if (prog_if == 0) {
                        kprintf("(VGA)\n");
                    } else if (prog_if == 1) {
                        kprintf("(8514)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x1:
                    kprintf("XGA Controller\n");
                    break;
                case 0x2:
                    kprintf("3D Controller\n");
                    break;
                case 0x80:
                    kprintf("Other Display Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_MULTIMEDIA:
            kprintf("Multimedia Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Multimedia Video Controller\n");
                    break;
                case 0x1:
                    kprintf("Multimedia Audio Controller\n");
                    break;
                case 0x2:
                    kprintf("Computer Telephony Device\n");
                    break;
                case 0x3:
                    kprintf("Audio Device\n");
                    break;
                case 0x80:
                    kprintf("Other Multimedia Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_MEMORY:
            kprintf("Memory Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("RAM Controller\n");
                    break;
                case 0x1:
                    kprintf("Flash Controller\n");
                    break;
                case 0x80:
                    kprintf("Other Memory Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_BRIDGE:
            kprintf("Bridge: ");
            switch (subclass_id) {
                case 0x00:
                    kprintf("Host Bridge\n");
                    break;
                case 0x01:
                    kprintf("ISA Bridge\n");
                    break;
                case 0x02:
                    kprintf("EISA Bridge\n");
                    break;
                case 0x03:
                    kprintf("MCA Bridge\n");
                    break;
                case 0x04:
                    kprintf("PCI-to-PCI Bridge ");
                    if (prog_if == 0) {
                        kprintf("(Normal Decode)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Subtractive Decode)\n");
                    } else {
                        kprintf("(Unknown Decode)\n");
                    }
                    break;
                case 0x05:
                    kprintf("PCMCIA Bridge\n");
                    break;
                case 0x06:
                    kprintf("NuBus Bridge\n");
                    break;
                case 0x07:
                    kprintf("CardBus Bridge\n");
                    break;
                case 0x08:
                    kprintf("RACEway Bridge ");
                    if (prog_if == 0) {
                        kprintf("(Transparent Mode)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Endpoint Mode)\n");
                    } else {
                        kprintf("(Unknown Mode)\n");
                    }
                    break;
                case 0x09:
                    kprintf("PCI-to-PCI Bridge");
                    if (prog_if == 0x40) {
                        kprintf(
                            "(Semi-Transparent, Primary Bus towards Host)\n");
                    } else if (prog_if == 0x80) {
                        kprintf(
                            "(Semi-Transparent, Secondary Bus towards Host)\n");
                    } else {
                        kprintf("(Unknown Mode)\n");
                    }
                    break;
                case 0x0A:
                    kprintf("InfiniBand-to-PCI Host Bridge\n");
                    break;
                case 0x80:
                    kprintf("Other Bridge Device\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_SIMPLE_COMM:
            kprintf("Simple Communication Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Serial Controller ");
                    if (prog_if == 0) {
                        kprintf("(8250-Compatible, Generic XT)\n");
                    } else if (prog_if == 1) {
                        kprintf("(16450-Compatible)\n");
                    } else if (prog_if == 2) {
                        kprintf("(16550-Compatible)\n");
                    } else if (prog_if == 3) {
                        kprintf("(16650-Compatible)\n");
                    } else if (prog_if == 4) {
                        kprintf("(16750-Compatible)\n");
                    } else if (prog_if == 5) {
                        kprintf("(16850-Compatible)\n");
                    } else if (prog_if == 6) {
                        kprintf("(16950-Compatible)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x1:
                    kprintf("Parallel Controller ");
                    if (prog_if == 0) {
                        kprintf("(Standard Parallel Port)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Bi-Directional Parallel Port)\n");
                    } else if (prog_if == 2) {
                        kprintf("(ECP 1.X Compliant Parallel Port)\n");
                    } else if (prog_if == 3) {
                        kprintf("(IEEE 1284 Controller)\n");
                    } else if (prog_if == 0xFE) {
                        kprintf("(IEEE 1284 Target Device)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x2:
                    kprintf("Multiport Serial Controller\n");
                    break;
                case 0x3:
                    kprintf("Modem ");
                    if (prog_if == 0) {
                        kprintf("(Generic Modem)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Hayes 16450-Compatible Interface)\n");
                    } else if (prog_if == 2) {
                        kprintf("(Hayes 16550-Compatible Interface)\n");
                    } else if (prog_if == 3) {
                        kprintf("(Hayes 16650-Compatible Interface)\n");
                    } else if (prog_if == 4) {
                        kprintf("(Hayes 16750-Compatible Interface)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x4:
                    kprintf("IEEE 488.1/488.2 (GPIB) Controller\n");
                    break;
                case 0x5:
                    kprintf("Smart Card\n");
                    break;
                case 0x80:
                    kprintf("Other Communication Device\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_BASE_SYSTEM_PERIPHERAL:
            kprintf("Base System Peripheral: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Programmable Interrupt Controller ");
                    if (prog_if == 0) {
                        kprintf("(Generic 8259-Compatible)\n");
                    } else if (prog_if == 0x1) {
                        kprintf("(ISA-Compatible)\n");
                    } else if (prog_if == 0x2) {
                        kprintf("(EISA-Compatible)\n");
                    } else if (prog_if == 0x10) {
                        kprintf("(I/O APIC Interrupt Controller)\n");
                    } else if (prog_if == 0x20) {
                        kprintf("(I/O(x) APIC Interrupt Controller)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x1:
                    kprintf("DMA Controller ");
                    if (prog_if == 0) {
                        kprintf("(Generic 8237-Compatible)\n");
                    } else if (prog_if == 1) {
                        kprintf("(ISA-Compatible)\n");
                    } else if (prog_if == 2) {
                        kprintf("(EISA-Compatible)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x2:
                    kprintf("Timer ");
                    if (prog_if == 0) {
                        kprintf("(Generic 8254-Compatible)\n");
                    } else if (prog_if == 1) {
                        kprintf("(ISA-Compatible)\n");
                    } else if (prog_if == 2) {
                        kprintf("(EISA-Compatible)\n");
                    } else if (prog_if == 3) {
                        kprintf("(HPET)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x3:
                    kprintf("RTC Controller ");
                    if (prog_if == 0) {
                        kprintf("(Generic RTC)\n");
                    } else if (prog_if == 1) {
                        kprintf("(ISA-Compatible)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x4:
                    kprintf("PCI Hot-Plug Controller\n");
                    break;
                case 0x5:
                    kprintf("SD Host Controller\n");
                    break;
                case 0x6:
                    kprintf("IOMMU\n");
                    break;
                case 0x80:
                    kprintf("Other System Peripheral\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_INPUT_DEVICE:
            kprintf("Input Device: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Keyboard Controller\n");
                    break;
                case 0x1:
                    kprintf("Digitizer Pen\n");
                    break;
                case 0x2:
                    kprintf("Mouse Controller\n");
                    break;
                case 0x3:
                    kprintf("Scanner Controller\n");
                    break;
                case 0x4:
                    kprintf("Gameport Controller ");
                    if (prog_if == 0x0) {
                        kprintf("(Generic)\n");
                    } else if (prog_if == 0x10) {
                        kprintf("(Extended)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x80:
                    kprintf("Other Input Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_DOCKING_STATION:
            kprintf("Docking Station: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Generic\n");
                    break;
                case 0x80:
                    kprintf("Other\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_PROCESSOR:
            kprintf("Processor: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("386\n");
                    break;
                case 0x1:
                    kprintf("486\n");
                    break;
                case 0x2:
                    kprintf("Pentium\n");
                    break;
                case 0x3:
                    kprintf("Pentium Pro\n");
                    break;
                case 0x10:
                    kprintf("Alpha\n");
                    break;
                case 0x20:
                    kprintf("PowerPC\n");
                    break;
                case 0x30:
                    kprintf("MIPS\n");
                    break;
                case 0x40:
                    kprintf("Co-Processor\n");
                    break;
                case 0x80:
                    kprintf("Other\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_SERIAL_BUS:
            kprintf("Serial Bus Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("FireWire (IEEE 1394) Controller ");
                    if (prog_if == 0x0) {
                        kprintf("(Generic)\n");
                    } else if (prog_if == 0x10) {
                        kprintf("(OHCI)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x1:
                    kprintf("ACCESS Bus Controller\n");
                    break;
                case 0x2:
                    kprintf("SSA\n");
                    break;
                case 0x3:
                    kprintf("USB Controller ");
                    if (prog_if == 0x0) {
                        kprintf("(UHCI)\n");
                    } else if (prog_if == 0x10) {
                        kprintf("(OHCI)\n");
                    } else if (prog_if == 0x20) {
                        kprintf("(EHCI, USB2)\n");
                    } else if (prog_if == 0x30) {
                        kprintf("(XHCI, USB3)\n");
                    } else if (prog_if == 0x80) {
                        kprintf("(Unspecified)\n");
                    } else if (prog_if == 0xFE) {
                        kprintf("(USB Device, Not a host controller)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x4:
                    kprintf("Fibre Channel\n");
                    break;
                case 0x5:
                    kprintf("SMBus Controller\n");
                    break;
                case 0x6:
                    kprintf("InfiniBand Controller\n");
                    break;
                case 0x7:
                    kprintf("IPMI Interface ");
                    if (prog_if == 0) {
                        kprintf("(SMIC)\n");
                    } else if (prog_if == 1) {
                        kprintf("(Keyboard Controller Style)\n");
                    } else if (prog_if == 2) {
                        kprintf("(Block Transfer)\n");
                    } else {
                        kprintf("(Unknown)\n");
                    }
                    break;
                case 0x8:
                    kprintf("SERCOS Interface (IEC 61491)\n");
                    break;
                case 0x9:
                    kprintf("CANbus Controller\n");
                    break;
                case 0x80:
                    kprintf("Other Bus Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_WIRELESS:
            kprintf("Wireless Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("iRDA Compatible Controller\n");
                    break;
                case 0x1:
                    kprintf("Consumer IR Controller\n");
                    break;
                case 0x10:
                    kprintf("RF Controller\n");
                    break;
                case 0x11:
                    kprintf("Bluetooth Controller\n");
                    break;
                case 0x12:
                    kprintf("Broadband Controller\n");
                    break;
                case 0x20:
                    kprintf("Ethernet Controller (802.11a)\n");
                    break;
                case 0x21:
                    kprintf("Ethernet Controller (802.11b)\n");
                    break;
                case 0x80:
                    kprintf("Other Wireless Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_INTELLIGENT_IO:
            kprintf("Intelligent I/O Controller\n");
            break;
        case PCI_CLASS_CODE_SATELLITE_COMM:
            kprintf("Satellite Communication Controller: ");
            switch (subclass_id) {
                case 0x1:
                    kprintf("TV Controller\n");
                    break;
                case 0x2:
                    kprintf("Audio Controller\n");
                    break;
                case 0x3:
                    kprintf("Voice Controller\n");
                    break;
                case 0x4:
                    kprintf("Data Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_ENCRYPTION:
            kprintf("Encryption Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("Network and Computing Encrpytion/Decryption\n");
                    break;
                case 0x10:
                    kprintf("Entertainment Encryption/Decryption\n");
                    break;
                case 0x80:
                    kprintf("Other Encryption/Decryption\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_SIGNAL:
            kprintf("Signal Processing Controller: ");
            switch (subclass_id) {
                case 0x0:
                    kprintf("DPIO Modules\n");
                    break;
                case 0x1:
                    kprintf("Performance Counters\n");
                    break;
                case 0x10:
                    kprintf("Communication Synchronizer\n");
                    break;
                case 0x20:
                    kprintf("Signal Processing Management\n");
                    break;
                case 0x80:
                    kprintf("Other Signal Processing Controller\n");
                    break;
                default:
                    kprintf("Unknown Subclass: %x\n", subclass_id);
                    break;
            }
            break;
        case PCI_CLASS_CODE_PROCESSING_ACCELERATOR:
            kprintf("Processing Accelerator\n");
            break;
        case PCI_CLASS_CODE_NON_ESSENTIAL:
            kprintf("Non-Essential Instrumentation\n");
            break;
        case PCI_CLASS_CODE_COPROCESSOR:
            kprintf("Co-Processor\n");
            break;
        case PCI_CLASS_CODE_UNASSIGNED:
            kprintf("Unassigned Class\n");
            break;
        default:
            kprintf("Unknown Class: %x\n", class_id);
            break;
    }
}
