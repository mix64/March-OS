#include <lib/string.h>
#include <x86/asm.h>

void memset(void *addr, uint8 data, uint64 count) {
#ifdef __x64__
    stosb(addr, data, count);
#else
    for (uint64 i = 0; i < count; i++) {
        ((uint8 *)addr)[i] = data;
    }
#endif
}
