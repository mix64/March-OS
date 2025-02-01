#pragma once
#include <types.h>

void ide_wait();
void ide_read(uint32 sector, void *buf);