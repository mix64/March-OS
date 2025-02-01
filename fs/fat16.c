#include <fs/fat16.h>
#include <ide.h>
#include <kernel.h>
#include <mm.h>

void fat16_mbr_dump(FAT16_MBR *mbr);

void fat16() {
    FAT16_MBR *mbr = (FAT16_MBR *)kmalloc(sizeof(FAT16_MBR));
    ide_read(0, mbr);
    fat16_mbr_dump(mbr);

    if (mbr->bootsig != 0xAA55 || mbr->table[0].boot != 0x80) {
        panic("FAT16: Invalid MBR\n");
    }

    debugf("FAT16 BPB:\n");
    FAT16_BPB *bpb = (FAT16_BPB *)kmalloc(sizeof(FAT16_BPB));
    for (int i = 0; i < 4; i++) {
        if (mbr->table[i].type == 0) {
            continue;
        }
        ide_read(mbr->table[i].bpb_begin, bpb);
        debugf("  Partition %d:\n", i);
        debugf("    JMP: %x %x %x\n", bpb->jmp[0], bpb->jmp[1], bpb->jmp[2]);
        debugf("    OEM: %s\n", bpb->oem);
        debugf("    Bytes per sector: %x\n", bpb->bytes_per_sector);
        debugf("    Sectors per cluster: %x\n", bpb->sectors_per_cluster);
        debugf("    Reserved sectors: %x\n", bpb->reserved_sectors);
        debugf("    FATS: %x\n", bpb->fats);
        debugf("    Root entries: %x\n", bpb->root_entries);
        debugf("    Sectors: %x\n", bpb->sectors);
        debugf("    Media: %x\n", bpb->media);
        debugf("    Sectors per FAT: %x\n", bpb->sectors_per_fat);
        debugf("    Sectors per track: %x\n", bpb->sectors_per_track);
        debugf("    Heads: %x\n", bpb->heads);
        debugf("    Hidden sectors: %x\n", bpb->hidden_sectors);
        debugf("    Large sectors: %x\n", bpb->large_sectors);
        debugf("    Drive number: %x\n", bpb->drive_number);
        debugf("    Signature: %x\n", bpb->signature);
        debugf("    Volume ID: %x\n", bpb->volume_id);
        debugf("    Volume label: %s\n", bpb->volume_label);
        debugf("    Filesystem type: %s\n", bpb->filesystem_type);
    }

    kmfree(bpb);
    kmfree(mbr);
    return;
}

void fat16_mbr_dump(FAT16_MBR *mbr) {
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