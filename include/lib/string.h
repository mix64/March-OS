#pragma once
#include <types.h>

void memset(void *addr, uint8 data, uint64 count);
void memcpy(void *dst, void *src, uint64 count);
int memcmp(const uint8 *p, const uint8 *q, uint64 limit);
