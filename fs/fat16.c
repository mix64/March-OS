#include <fs/fat16.h>
#include <ide.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>

void fat16_dump_mbr(FAT16_MBR *mbr);
void fat16_dump_bpb(FAT16_BPB *bpb);
void fat16_dump_dir(FAT16_DIR_ENTRY *dir, uint32 entries);
void fat16_read_cluster(uint16 cluster, char *buf);
uint16 fat16_next_cluster(uint16 cluster);
void fat16_test();

static FAT16 fat16;

void fat16_init() {
    /*
        Read MBR
    */
    FAT16_MBR *mbr = (FAT16_MBR *)kmalloc(sizeof(FAT16_MBR));
    ide_read(0, mbr);
#ifdef __DEBUG__
    fat16_dump_mbr(mbr);
#endif
    // TODO: Support multiple partitions
    if (mbr->bootsig != 0xAA55 || mbr->table[0].boot != 0x80 ||
        (mbr->table[0].type != 0x04 && mbr->table[0].type != 0x06)) {
        panic("FAT16: Invalid MBR\n");
    }
    fat16.bpb_entry = mbr->table[0].bpb_begin;
    kprintf("[fs] FAT16 Partition0 found\n");

    /*
        Read BPB
    */
    FAT16_BPB *bpb = (FAT16_BPB *)kmalloc(sizeof(FAT16_BPB));
    ide_read(fat16.bpb_entry, bpb);
#ifdef __DEBUG__
    fat16_dump_bpb(bpb);
#endif
    if (bpb->bytes_per_sector != SECTOR_SIZE || bpb->fats != 2 ||
        (bpb->signature != 0x28 && bpb->signature != 0x29)) {
        panic("FAT16: Invalid BPB\n");
    }
    fat16.sector_size = bpb->bytes_per_sector;
    fat16.cluster_size = bpb->bytes_per_sector * bpb->sectors_per_cluster;
    fat16.fat_size = bpb->bytes_per_sector * bpb->sectors_per_fat;
    fat16.partition0_size = mbr->table[0].sectors * bpb->bytes_per_sector;
    fat16.sector_per_cluster = bpb->sectors_per_cluster;
    fat16.sector_per_fat = bpb->sectors_per_fat;
    fat16.sector_per_partition0 = bpb->sectors;
    fat16.fat_entry = fat16.bpb_entry + bpb->reserved_sectors;
    fat16.root_entries = bpb->root_entries;
    fat16.rootdir_entry = fat16.fat_entry + bpb->sectors_per_fat * bpb->fats;
    fat16.data_entry = fat16.rootdir_entry + sizeof(FAT16_DIR_ENTRY) *
                                                 bpb->root_entries /
                                                 bpb->bytes_per_sector;

    kprintf("[fs] Partition Size: %d KiB\n", fat16.partition0_size / KiB(1));
    kprintf("[fs] Cluster Size: %d KiB\n", fat16.cluster_size / KiB(1));
    kprintf("[fs] FAT Size: %d KiB\n", fat16.fat_size / KiB(1));
    kprintf("[fs] Volume Label: %s\n", bpb->volume_label);

    kmfree(mbr);
    kmfree(bpb);

    fat16_test();
    return;
}

void fat16_read_cluster(uint16 cluster, char *buf) {
    for (int i = 0; i < fat16.sector_per_cluster; i++) {
        ide_read(
            fat16.data_entry + (cluster - 2) * fat16.sector_per_cluster + i,
            buf + fat16.sector_size * i);
    }
}

uint16 fat16_next_cluster(uint16 cluster) {
    uint16 *fat = (uint16 *)kmalloc(fat16.sector_size);

    // 256 = 512(sector size) / 2(FAT entry size)
    uint16 fat_entries = fat16.sector_size / sizeof(uint16);
    ide_read(fat16.fat_entry + (cluster / fat_entries), fat);
    uint16 next = fat[cluster % fat_entries];

    kmfree(fat);
    return next;
}

void fat16_test() {
    // dump rootdir
    uint32 rootdir_size = sizeof(FAT16_DIR_ENTRY) * fat16.root_entries;
    FAT16_DIR_ENTRY *rootdir = (FAT16_DIR_ENTRY *)kmalloc(rootdir_size);
    ide_read_seq(fat16.rootdir_entry, rootdir,
                 rootdir_size / fat16.sector_size);
    debugf("rootdir entry: %x\n", fat16.rootdir_entry);
    debugf("rootdir size: %d\n", rootdir_size);

#ifdef __DEBUG__
    fat16_dump_dir(rootdir, fat16.root_entries);
#endif
    kmfree(rootdir);
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

void fat16_dump_dir(FAT16_DIR_ENTRY *dir, uint32 entries) {
    for (int i = 0; i < entries; i++) {
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
            kprintf("  LFN: %x\n", lfn->ord);
            kprintf("    filename: %s\n", filename);
            kprintf("    attr: %x\n", lfn->attr);
            kprintf("    checksum: %x\n", lfn->checksum);
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

        kprintf("  %s\n", filename);
        kprintf("    FileType: ");
        if (dir[i].attr & 0x01) {
            kprintf("Read-only ");
        }
        if (dir[i].attr & 0x02) {
            kprintf("Hidden ");
        }
        if (dir[i].attr & 0x04) {
            kprintf("System ");
        }
        if (dir[i].attr & 0x08) {
            kprintf("VolumeID ");
        }
        if (dir[i].attr & 0x10) {
            kprintf("Directory ");
        }
        if (dir[i].attr & 0x20) {
            kprintf("Archive ");
        }
        kprintf("(%x) \n", dir[i].attr);
        kprintf("    Cluster: %x\n", dir[i].cluster);
        kprintf("    Size: %d\n", dir[i].size);
        kprintf("    Create DateTime: %d-%d-%d %d:%d:%d\n", c_year, c_month,
                c_day, c_hour, c_minute, c_second);
        kprintf("    Last Modify DateTime: %d-%d-%d %d:%d:%d\n", m_year,
                m_month, m_day, m_hour, m_minute, m_second);
        kprintf("    Last Access Date: %d-%d-%d\n", a_year, a_month, a_day);
    }
}
