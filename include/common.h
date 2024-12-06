#pragma once

#define BIT64_MASK(n) ((1ULL << (n)) - 1)

#define KiB(x) (x << 10)
#define MiB(x) (x << 20)
#define GiB(x) (x << 30)

#define ROUNDUP(a, n) (((a) + (n) - 1) & ~((n) - 1))
#define ROUNDDOWN(a, n) ((a) & ~((n) - 1))