#include <ide.h>
#include <types.h>
#include <x86/asm.h>

#define IDE_COMMAND 0x1F7
#define ATA_SR_BSY 0x80   // Busy
#define ATA_SR_DRDY 0x40  // Drive ready
#define ATA_SR_DF 0x20    // Drive write fault
#define ATA_SR_DSC 0x10   // Drive seek complete
#define ATA_SR_DRQ 0x08   // Data request ready
#define ATA_SR_CORR 0x04  // Corrected data
#define ATA_SR_IDX 0x02   // Index
#define ATA_SR_ERR 0x01   // Error

void ide_wait() {
    while ((inb(IDE_COMMAND) & (ATA_SR_BSY | ATA_SR_DRDY)) != ATA_SR_DRDY);
}

void ide_read(uint32 sector, char *buf) {
    outb(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
    outb(0x1F2, 1);
    outb(0x1F3, sector & 0xFF);
    outb(0x1F4, (sector >> 8) & 0xFF);
    outb(0x1F5, (sector >> 16) & 0xFF);
    outb(0x1F7, 0x20);

    while (!(inb(0x1F7) & 0x08));
    for (int i = 0; i < 512 / 4; i++) {
        uint32 data = inl(0x1F0);
        *(uint32 *)buf = data;
        buf += 4;
    }
}