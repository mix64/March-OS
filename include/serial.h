#pragma once
#include <types.h>

int init_serial();
void kprint(char *str);
void _kprintf(char *fmt, uint8 argc, uintptr *argv);

#define KPRINTF_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define KPRINTF_ARG_COUNT(...) \
    KPRINTF_ARG_COUNT_IMPL(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define kprintf(fmt, ...)                                             \
    do {                                                              \
        uintptr _kprintf_args[] = {(uintptr)__VA_ARGS__};             \
        _kprintf(fmt, KPRINTF_ARG_COUNT(__VA_ARGS__), _kprintf_args); \
    } while (0)
