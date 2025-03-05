#pragma once
#include <types.h>

void ide_read(uint32 sector, void *buf);
void ide_read_seq(uint32 sector, void *buf, uint32 count);
