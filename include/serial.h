#pragma once
#include <types.h>

int init_serial();
void kprint(char *str);
void _kprintf(char *fmt, uint8 argc, uintptr *argv);

#define KPRINTF_ARG_COUNT(...) \
    KPRINTF_ARG_COUNT_IMPL(0, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)
#define KPRINTF_ARG_COUNT_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N

#define KPRINTF_CAST_ARGS_0()
#define KPRINTF_CAST_ARGS_1(a1) (uintptr)(a1)
#define KPRINTF_CAST_ARGS_2(a1, a2) (uintptr)(a1), (uintptr)(a2)
#define KPRINTF_CAST_ARGS_3(a1, a2, a3) \
    (uintptr)(a1), (uintptr)(a2), (uintptr)(a3)
#define KPRINTF_CAST_ARGS_4(a1, a2, a3, a4) \
    (uintptr)(a1), (uintptr)(a2), (uintptr)(a3), (uintptr)(a4)
#define KPRINTF_CAST_ARGS_5(a1, a2, a3, a4, a5) \
    (uintptr)(a1), (uintptr)(a2), (uintptr)(a3), (uintptr)(a4), (uintptr)(a5)
#define KPRINTF_CAST_ARGS_6(a1, a2, a3, a4, a5, a6)                            \
    (uintptr)(a1), (uintptr)(a2), (uintptr)(a3), (uintptr)(a4), (uintptr)(a5), \
        (uintptr)(a6)
#define KPRINTF_CAST_ARGS_7(a1, a2, a3, a4, a5, a6, a7)                        \
    (uintptr)(a1), (uintptr)(a2), (uintptr)(a3), (uintptr)(a4), (uintptr)(a5), \
        (uintptr)(a6), (uintptr)(a7)

#define KPRINTF_CAT(a, b) KPRINTF_CAT_IMPL(a, b)
#define KPRINTF_CAT_IMPL(a, b) a##b

#define KPRINTF_EXPAND(x) x
#define KPRINTF_CAST_ARGS_N(N, ...) \
    KPRINTF_EXPAND(KPRINTF_CAT(KPRINTF_CAST_ARGS_, N))(__VA_ARGS__)

#define kprintf(fmt, ...)                                                    \
    do {                                                                     \
        uintptr _kprintf_args[] = {KPRINTF_CAST_ARGS_N(                      \
            KPRINTF_ARG_COUNT(__VA_ARGS__), ##__VA_ARGS__)};                 \
        _kprintf(fmt, KPRINTF_ARG_COUNT(__VA_ARGS__),                        \
                 KPRINTF_ARG_COUNT(__VA_ARGS__) > 0 ? _kprintf_args : NULL); \
    } while (0)

#define panic(fmt, ...)              \
    do {                             \
        kprintf("panic: ");          \
        kprintf(fmt, ##__VA_ARGS__); \
        while (1);                   \
    } while (0)

#ifdef __DEBUG__
#define debugf(fmt, ...) kprintf(fmt, ##__VA_ARGS__)
#else
#define debugf(fmt, ...)
#endif