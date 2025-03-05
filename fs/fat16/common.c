#include <fs/fat16.h>
#include <ide.h>
#include <mm.h>

void fat16_read_rootdir(void *buf) {
    ide_read_seq(
        fat16.rootdir_entry, buf,
        sizeof(FAT16_DIR_ENTRY) * fat16.root_entries / fat16.sector_size);
}

void fat16_read_cluster(uint16 cluster, void *buf) {
    ide_read_seq(fat16.data_entry + (cluster - 2) * fat16.sector_per_cluster,
                 buf, fat16.sector_per_cluster);
}

uint16 fat16_find_cluster(char *filename, uint16 dir_cluster) {
    FAT16_DIR_ENTRY *dir;
    uint16 entries;
    if (dir_cluster == 0) {
        entries = fat16.root_entries;
        dir = (FAT16_DIR_ENTRY *)kmalloc(sizeof(FAT16_DIR_ENTRY) * entries);
        fat16_read_rootdir(dir);
    } else {
        entries = fat16.cluster_size / sizeof(FAT16_DIR_ENTRY);
        dir = (FAT16_DIR_ENTRY *)kmalloc(fat16.cluster_size);
        fat16_read_cluster(dir_cluster, (char *)dir);
    }

    for (int i = 0; i < entries; i++) {
        if (dir[i].filename[0] == 0x00) {
            continue;
        }
        if (dir[i].attr == 0x0F) {
            continue;
        }
        char entry_filename[12];
        for (int j = 0; j < 11; j++) {
            entry_filename[j] = dir[i].filename[j];
            if (entry_filename[j] == ' ') {
                entry_filename[j] = '\0';
            }
        }
        entry_filename[11] = '\0';
        if (strcmp(filename, entry_filename) == 0) {
            kmfree(dir);
            return dir[i].cluster;
        }
    }

    kmfree(dir);
    return 0;
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
