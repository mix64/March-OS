#include <ide.h>
#include <kernel.h>
#include <x86/asm.h>
#include <x86/ata.h>

void ide_wait_ready() {
    uint64 retries = 10000;
    while (retries--) {
        if ((inb(IDE_COMMAND) & (ATA_SR_BSY | ATA_SR_DRDY)) == ATA_SR_DRDY) {
            return;
        }
    }
    panic("IDE: Wait Ready Timeout\n");
}

void ide_wait_data() {
    uint64 retries = 10000;
    while (retries--) {
        if (inb(IDE_COMMAND) & ATA_SR_DRQ) {
            return;
        }
    }
    panic("IDE: Wait Data Timeout\n");
}

void ide_read_seq(uint32 sector, void *buf, uint32 count) {
    ide_wait_ready();
    outb(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
    outb(0x1F2, count);
    outb(0x1F3, sector & 0xFF);
    outb(0x1F4, (sector >> 8) & 0xFF);
    outb(0x1F5, (sector >> 16) & 0xFF);
    outb(IDE_COMMAND, ATA_CMD_READ_PIO);

    for (int i = 0; i < count; i++) {
        ide_wait_data();
        for (int j = 0; j < 512; j += 4) {
            *(uint32 *)buf = inl(0x1F0);
            buf += 4;
        }
    }
}

void ide_read(uint32 sector, void *buf) { ide_read_seq(sector, buf, 1); }