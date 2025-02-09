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

void fat16_dump_dir(FAT16_DIR_ENTRY *dir) {
    debugf("FAT16 DIR:\n");
    for (int i = 0; i < 16; i++) {
        if (dir[i].filename[0] == 0x00) {
            break;
        }
        if (dir[i].attr == 0x0F) {
            FAT16_LFN_ENTRY *lfn = (FAT16_LFN_ENTRY *)&dir[i];
            char filename[14];
            for (int j = 0; j < 5; j++) {
                filename[j] = (char)lfn->name1[j];
            }
            for (int j = 0; j < 6; j++) {
                filename[j + 5] = (char)lfn->name2[j];
            }
            for (int j = 0; j < 2; j++) {
                filename[j + 11] = (char)lfn->name3[j];
            }
            filename[13] = '\0';
            debugf("  LFN: %x\n", lfn->ord);
            debugf("    filename: %s\n", filename);
            debugf("    attr: %x\n", lfn->attr);
            debugf("    type: %x\n", lfn->type);
            debugf("    checksum: %x\n", lfn->checksum);
            continue;
        }
        char filename[12];
        memcpy(filename, dir[i].filename, 11);
        filename[11] = '\0';
        uint16 c_year = ((dir[i].cdate & 0xFE00) >> 9) + 1980;
        uint8 c_month = (dir[i].cdate & 0x01E0) >> 5;
        uint8 c_day = dir[i].cdate & 0x001F;
        uint8 c_hour = (dir[i].ctime & 0xF800) >> 11;
        uint8 c_minute = (dir[i].ctime & 0x07E0) >> 5;
        uint8 c_second = (dir[i].ctime & 0x001F) * 2 + dir[i].ctime_ms / 100;
        uint16 m_year = ((dir[i].mdate & 0xFE00) >> 9) + 1980;
        uint8 m_month = (dir[i].mdate & 0x01E0) >> 5;
        uint8 m_day = dir[i].mdate & 0x001F;
        uint8 m_hour = (dir[i].mtime & 0xF800) >> 11;
        uint8 m_minute = (dir[i].mtime & 0x07E0) >> 5;
        uint8 m_second = (dir[i].mtime & 0x001F) * 2;
        uint16 a_year = ((dir[i].adate & 0xFE00) >> 9) + 1980;
        uint8 a_month = (dir[i].adate & 0x01E0) >> 5;
        uint8 a_day = dir[i].adate & 0x001F;

        debugf("  %s\n", filename);
        debugf("    Attr: %x\n", dir[i].attr);
        debugf("    Cluster: %x\n", dir[i].cluster);
        debugf("    Size: %d\n", dir[i].size);
        debugf("    Create DateTime: %d-%d-%d %d:%d:%d\n", c_year, c_month,
               c_day, c_hour, c_minute, c_second);
        debugf("    Last Modify DateTime: %d-%d-%d %d:%d:%d\n", m_year, m_month,
               m_day, m_hour, m_minute, m_second);
        debugf("    Last Access Date: %d-%d-%d\n", a_year, a_month, a_day);
    }
}
