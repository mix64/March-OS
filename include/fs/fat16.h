#pragma once
#include <fs/inode.h>
#include <types.h>

/* references:
    https://wiki.osdev.org/FAT
    http://resove.cool.coocan.jp/fatKnowledge/precision.html
    https://www.adrian.idv.hk/2009-11-15-fat32/
    https://www.markn.org/blog/2008/07/sd2.html
*/

#define FAT16_ATTR_READONLY 0x01
#define FAT16_ATTR_HIDDEN 0x02
#define FAT16_ATTR_SYSTEM 0x04
#define FAT16_ATTR_VOLUMEID 0x08
#define FAT16_ATTR_DIRECTORY 0x10
#define FAT16_ATTR_ARCHIVE 0x20
#define FAT16_ATTR_LFN 0x0F

typedef struct {
    uint16 bpb_entry;
    uint16 fat_entry;
    uint16 rootdir_entry;
    uint16 data_entry;

    uint16 sector_size;
    uint16 cluster_size;
    uint16 fat_size;
    uint32 partition0_size;

    uint16 sector_per_cluster;
    uint16 sector_per_fat;
    uint16 sector_per_partition0;

    uint16 root_entries;

} FAT16;

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

typedef struct __attribute__((packed)) {
    char filename[11];
    uint8 attr;  // 0x01: read-only, 0x02: hidden, 0x04: system, 0x08: volumeID,
                 // 0x10: subdirectory, 0x20: archive, 0x0f: Long filename
    uint8 _reserved;  // for Windows NT
    uint8 ctime_ms;
    uint16 ctime;
    uint16 cdate;
    uint16 adate;
    uint16 _reserved2;  // for FAT32
    uint16 mtime;
    uint16 mdate;
    uint16 cluster;
    uint32 size;
} FAT16_DIR_ENTRY;

typedef struct __attribute__((packed)) {
    uint8 ord;
    short name1[5];
    uint8 attr;
    uint8 _zero0;  // 0
    uint8 checksum;
    short name2[6];
    uint16 _zero1;
    short name3[2];
} FAT16_LFN_ENTRY;

void fat16_init();
inode_t* fat16_namei(char* path);
int fat16_readi(inode_t* ino, void* dst, uint64 offset, uint64 size);
