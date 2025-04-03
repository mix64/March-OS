#include <acpi.h>
#include <apic.h>
#include <kernel.h>
#include <lib/string.h>
#include <system.h>

extern APIC apic;  // x86/apic.c

void dump_xsdp(ACPI_XSDP *xsdp);
void dump_xsdt(ACPI_XSDT_HEADER *xsdt);
void dump_madt(ACPI_MADT *madt);

void acpi_init(uintptr xsdp_addr) {
    ACPI_XSDP *xsdp = (ACPI_XSDP *)xsdp_addr;
    dump_xsdp(xsdp);

    ACPI_XSDT_HEADER *xsdt = (ACPI_XSDT_HEADER *)xsdp->XsdtAddress;
    dump_xsdt(xsdt);

    uintptr *xsdt_table = (uintptr *)((uintptr)xsdt + sizeof(ACPI_XSDT_HEADER));
    uint8 xsdt_n = (xsdt->Length - sizeof(ACPI_XSDT_HEADER)) / sizeof(uintptr);
    debugf("[acpi] XSDT Entries: %d\n", xsdt_n);

    for (uint8 i = 0; i < xsdt_n; i++) {
        ACPI_XSDT_HEADER *entry = (ACPI_XSDT_HEADER *)xsdt_table[i];
        char sig[5] = {0};
        memcpy(sig, entry->Signature, 4);
        if (!strcmp(sig, "APIC")) {
            ACPI_MADT *madt = (ACPI_MADT *)entry;
            dump_madt(madt);
            apic.lapic_addr = madt->LocalAPICAddress;

            ACPI_MADT_HEAD *madt_entry =
                (ACPI_MADT_HEAD *)((uintptr)madt + sizeof(ACPI_MADT));
            int remain = (int)(madt->Length - sizeof(ACPI_MADT));
            while (remain > 0) {
                int len = 0;
                switch (madt_entry->entry_type) {
                    case ACPI_MADT_T_LAPIC:
                        len = sizeof(ACPI_MADT_LAPIC);
                        ACPI_MADT_LAPIC *lapic = (ACPI_MADT_LAPIC *)madt_entry;
                        debugf("  LAPIC:\n");
                        debugf("    ACPI Processor ID: %d\n",
                               lapic->acpi_processor_id);
                        debugf("    Local APIC ID: %d\n", lapic->apic_id);
                        debugf("    Flags: %x\n", lapic->flags);

                        // Store LAPIC information in the global APIC structure
                        apic.lapic_id = lapic->apic_id;
                        break;

                    case ACPI_MADT_T_IOAPIC:
                        len = sizeof(ACPI_MADT_IOAPIC);
                        ACPI_MADT_IOAPIC *ioapic =
                            (ACPI_MADT_IOAPIC *)madt_entry;
                        debugf("  IOAPIC:\n");
                        debugf("    IOAPIC ID: %d\n", ioapic->ioapic_id);
                        debugf("    IOAPIC Address: %x\n",
                               ioapic->ioapic_address);
                        debugf("    Global System Interrupt Base: %d\n",
                               ioapic->global_system_interrupt_base);

                        // Store IOAPIC information in the global APIC structure
                        apic.ioapic_id = ioapic->ioapic_id;
                        apic.ioapic_addr = ioapic->ioapic_address;
                        break;

                    case ACPI_MADT_T_INTERRUPT_OVERRIDE:
                        len = sizeof(ACPI_MADT_INTERRUPT_OVERRIDE);
                        ACPI_MADT_INTERRUPT_OVERRIDE *intr =
                            (ACPI_MADT_INTERRUPT_OVERRIDE *)madt_entry;
                        debugf("  Interrupt Override:\n");
                        debugf("    Bus: %d\n", intr->bus_source);
                        debugf("    Source: %d\n", intr->irq_source);
                        debugf("    Global System Interrupt: %d\n",
                               intr->global_system_interrupt);
                        debugf("    Flags: %x\n", intr->flags);
                        break;

                    case ACPI_MADT_T_NMI:
                        len = sizeof(ACPI_MADT_NMI);
                        ACPI_MADT_NMI *nmi = (ACPI_MADT_NMI *)madt_entry;
                        debugf("  NMI:\n");
                        debugf("    NMI Source: %d\n", nmi->nmi_source);
                        debugf("    Global System Interrupt: %d\n",
                               nmi->global_system_interrupt);
                        debugf("    Flags: %x\n", nmi->flags);
                        break;

                    case ACPI_MADT_T_LAPIC_NMI:
                        len = sizeof(ACPI_MADT_LAPIC_NMI);
                        ACPI_MADT_LAPIC_NMI *lapic_nmi =
                            (ACPI_MADT_LAPIC_NMI *)madt_entry;
                        debugf("  LAPIC NMI:\n");
                        debugf("    ACPI Processor ID: %d\n",
                               lapic_nmi->acpi_processor_id);
                        debugf("    Flags: %x\n", lapic_nmi->flags);
                        debugf("    LINT: %d\n", lapic_nmi->lint);
                        break;

                    case ACPI_MADT_T_LAPIC_ADDRESS_OVERRIDE:
                        len = sizeof(ACPI_MADT_LAPIC_ADDRESS_OVERRIDE);
                        ACPI_MADT_LAPIC_ADDRESS_OVERRIDE *lapic_override =
                            (ACPI_MADT_LAPIC_ADDRESS_OVERRIDE *)madt_entry;
                        debugf("  LAPIC Address Override:\n");
                        debugf("    Local APIC Address: %llx\n",
                               lapic_override->local_apic_address);

                        // Override the LAPIC address in the global APIC
                        // structure
                        apic.lapic_addr = lapic_override->local_apic_address;
                        break;

                    case ACPI_MADT_T_LOCAL_X2APIC:
                        len = sizeof(ACPI_MADT_LOCAL_X2APIC);
                        ACPI_MADT_LOCAL_X2APIC *x2apic =
                            (ACPI_MADT_LOCAL_X2APIC *)madt_entry;
                        debugf("  Local x2APIC:\n");
                        debugf("    x2APIC ID: %d\n", x2apic->local_x2apic_id);
                        debugf("    ACPI ID: %d\n", x2apic->acpi_id);
                        debugf("    Flags: %x\n", x2apic->flags);
                        break;

                    default:
                        panic("[acpi] Unknown MADT type: %d\n",
                              madt_entry->entry_type);
                }
                remain -= len;
                madt_entry = (ACPI_MADT_HEAD *)((uintptr)madt_entry + len);
            }
        }
    }
}

void dump_xsdp(ACPI_XSDP *xsdp) {
    char sig[9] = {0};
    char oem[9] = {0};
    memcpy(sig, xsdp->Signature, 8);
    memcpy(oem, xsdp->OEMID, 6);
    debugf("[acpi] XSDP:\n");
    debugf("  Signature: %s\n", sig);
    debugf("  OEMID: %s\n", oem);
    debugf("  Revision: %d\n", xsdp->Revision);
    debugf("  RsdtAddress: %x\n", xsdp->RsdtAddress);
    debugf("  XsdtAddress: %x\n", xsdp->XsdtAddress);
}

void dump_xsdt(ACPI_XSDT_HEADER *xsdt) {
    char sig[9] = {0};
    char oem[9] = {0};
    memcpy(sig, xsdt->Signature, 8);
    memcpy(oem, xsdt->OEMID, 6);
    debugf("[acpi] XSDT:\n");
    debugf("  Signature: %s\n", sig);
    debugf("  OEMID: %s\n", oem);
    debugf("  Revision: %d\n", xsdt->Revision);
    debugf("  OEMRevision: %d\n", xsdt->OEMRevision);
}

void dump_madt(ACPI_MADT *madt) {
    debugf("[acpi] MADT:\n");
    debugf("  Revision: %d\n", madt->Revision);
    debugf("  LocalAPICAddress: %x\n", madt->LocalAPICAddress);
    debugf("  Flags: %x\n", madt->Flags);
}
