#pragma once
#include <types.h>

/* references:
    https://wiki.osdev.org/FAT
    http://resove.cool.coocan.jp/fatKnowledge/precision.html
    https://www.adrian.idv.hk/2009-11-15-fat32/
    https://www.markn.org/blog/2008/07/sd2.html
*/

typedef struct __attribute__((packed)) {
    uint8 _code[446];
    struct {
        uint8 boot;
        uint8 chs_begin[3];
        uint8 type;
        uint8 chs_end[3];
        uint32 bpb_begin;
        uint32 sectors;
    } table[4];
    uint16 bootsig;  // 0x55AA
} FAT16_MBR;

typedef struct __attribute__((packed)) {
    uint8 jmp[3];  // 0xEB 0x3C 0x90 (The 3C value may be different.)
    char oem[8];
    uint16 bytes_per_sector;
    uint8 sectors_per_cluster;
    uint16 reserved_sectors;
    uint8 fats;  // 2
    uint16 root_entries;
    uint16 sectors;
    uint8 media;  // HDD: 0xF8, Floppy: 0xF0
    uint16 sectors_per_fat;
    uint16 sectors_per_track;
    uint16 heads;
    uint32 hidden_sectors;
    uint32 large_sectors;
    uint8 drive_number;  // HDD: 0x80, Floppy: 0x00
    uint8 _reserved;
    uint8 signature;  // 0x28 or 0x29
    uint32 volume_id;
    char volume_label[11];
    char filesystem_type[8];
    uint8 _code[448];
    uint16 bootsig;  // 0x55AA
} FAT16_BPB;

void fat16();
