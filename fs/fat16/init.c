#include <fs/fat16.h>
#include <ide.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm.h>
#include <vfs.h>

extern void fat16_dump_mbr(FAT16_MBR *mbr);
extern void fat16_dump_bpb(FAT16_BPB *bpb);

FAT16 fat16;

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
    return;
}
