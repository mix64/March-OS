#pragma once
#include <types.h>

void memset(void *addr, uint8 data, uint64 count);
void memcpy(void *dst, void *src, uint64 count);
int memcmp(const void *p, const void *q, uint64 limit);
int strcmp(const char *dest, const char *src);
