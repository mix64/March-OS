#include <lib/string.h>
#include <x86/asm.h>

void memset(void *addr, uint8 data, uint64 count) {
#ifdef __x86__
    stosb(addr, data, count);
#else
    for (uint64 i = 0; i < count; i++) {
        ((uint8 *)addr)[i] = data;
    }
#endif
}

void memcpy(void *dest, void *src, uint64 count) {
#ifdef __x86__
    movsb(dest, src, count);
#else
    for (uint64 i = 0; i < count; i++) {
        ((uint8 *)dest)[i] = ((uint8 *)src)[i];
    }
#endif
}

int memcmp(const void *p, const void *q, uint64 limit) {
    for (uint64 i = 0; i < limit; i++) {
        if (((uint8 *)p)[i] != ((uint8 *)q)[i]) {
            return 1;
        }
    }
    return 0;
}

int strcmp(const char *dest, const char *src) {
    while (*dest != '\0' && *src != '\0') {
        if (*dest != *src) {
            return 1;
        }
        dest++;
        src++;
    }
    if (*dest != *src) {
        return 1;
    }
    return 0;
}