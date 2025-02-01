#include <fs/fat16.h>
#include <ide.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>

void fat16_dump_mbr(FAT16_MBR *mbr);
void fat16_dump_bpb(FAT16_BPB *bpb);

void fat16() {
    FAT16_MBR *mbr = (FAT16_MBR *)kmalloc(sizeof(FAT16_MBR));
    ide_read(0, mbr);
    fat16_dump_mbr(mbr);

    if (mbr->bootsig != 0xAA55 || mbr->table[0].boot != 0x80 ||
        (mbr->table[0].type != 0x04 && mbr->table[0].type != 0x06)) {
        panic("FAT16: Invalid MBR\n");
    }

    uint32 bpb_begin = mbr->table[0].bpb_begin;
    kprintf("[fs] FAT16 Partition found\n");
    kprintf("[fs] BIOS Parameter Block: %x\n", bpb_begin);

    FAT16_BPB *bpb = (FAT16_BPB *)kmalloc(sizeof(FAT16_BPB));
    // Only the first partition is supported
    ide_read(bpb_begin, bpb);
    fat16_dump_bpb(bpb);
    if (bpb->bytes_per_sector != 512 || bpb->fats != 2 ||
        (bpb->signature != 0x28 && bpb->signature != 0x29)) {
        panic("FAT16: Invalid BPB\n");
    }
    kprintf("[fs] Volume Label: %s\n", bpb->volume_label);

    uint32 fat_begin = bpb_begin + bpb->reserved_sectors;
    uint32 root_begin = fat_begin + bpb->sectors_per_fat * bpb->fats;
    kprintf("[fs] File Allocation Table: %x - %x\n", fat_begin, root_begin - 1);

    FAT16_DIR_ENTRY *dir =
        (FAT16_DIR_ENTRY *)kmalloc(sizeof(FAT16_DIR_ENTRY) * 16);
    ide_read(root_begin, dir);
    debugf("FAT16 DIR:\n");
    for (int i = 0; i < 16; i++) {
        if (dir[i].filename[0] == 0x00) {
            break;
        }
        char filename[12];
        memcpy(filename, dir[i].filename, 11);
        filename[11] = '\0';
        debugf("  %s\n", filename);
        debugf("    Attr: %x\n", dir[i].attr);
        debugf("    CTimeMS: %x\n", dir[i].ctime_ms);
        debugf("    CTime: %x\n", dir[i].ctime);
        debugf("    CDate: %x\n", dir[i].cdate);
        debugf("    ADate: %x\n", dir[i].adate);
        debugf("    MTime: %x\n", dir[i].mtime);
        debugf("    MDate: %x\n", dir[i].mdate);
        debugf("    Cluster: %x\n", dir[i].cluster);
        debugf("    Size: %x\n", dir[i].size);
    }

    kmfree(bpb);
    kmfree(mbr);
    kmfree(dir);
    return;
}

void fat16_dump_mbr(FAT16_MBR *mbr) {
    debugf("FAT16 MBR:\n");
    for (int i = 0; i < 4; i++) {
        if (mbr->table[i].type == 0) {
            continue;
        }
        debugf("  Partition %d:\n", i);
        debugf("    Bootable: %x\n", mbr->table[i].boot);
        debugf("    CHS Begin: %x %x %x\n", mbr->table[i].chs_begin[0],
               mbr->table[i].chs_begin[1], mbr->table[i].chs_begin[2]);
        debugf("    Type: %x\n", mbr->table[i].type);
        debugf("    CHS End: %x %x %x\n", mbr->table[i].chs_end[0],
               mbr->table[i].chs_end[1], mbr->table[i].chs_end[2]);
        debugf("    BPB Begin: %x\n", mbr->table[i].bpb_begin);
        debugf("    Sectors: %x\n", mbr->table[i].sectors);
    }
}

void fat16_dump_bpb(FAT16_BPB *bpb) {
    debugf("FAT16 BPB:\n");
    debugf("  JMP: %x %x %x\n", bpb->jmp[0], bpb->jmp[1], bpb->jmp[2]);
    debugf("  OEM: %s\n", bpb->oem);
    debugf("  Bytes per sector: %x\n", bpb->bytes_per_sector);
    debugf("  Sectors per cluster: %x\n", bpb->sectors_per_cluster);
    debugf("  Reserved sectors: %x\n", bpb->reserved_sectors);
    debugf("  FATS: %x\n", bpb->fats);
    debugf("  Root entries: %x\n", bpb->root_entries);
    debugf("  Sectors: %x\n", bpb->sectors);
    debugf("  Media: %x\n", bpb->media);
    debugf("  Sectors per FAT: %x\n", bpb->sectors_per_fat);
    debugf("  Sectors per track: %x\n", bpb->sectors_per_track);
    debugf("  Heads: %x\n", bpb->heads);
    debugf("  Hidden sectors: %x\n", bpb->hidden_sectors);
    debugf("  Large sectors: %x\n", bpb->large_sectors);
    debugf("  Drive number: %x\n", bpb->drive_number);
    debugf("  Signature: %x\n", bpb->signature);
    debugf("  Volume ID: %x\n", bpb->volume_id);
    debugf("  Volume label: %s\n", bpb->volume_label);
    debugf("  Filesystem type: %s\n", bpb->filesystem_type);
}