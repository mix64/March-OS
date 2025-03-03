#include <serial.h>

extern void serial_putchar(char c);

void putchar(char c) { serial_putchar(c); }

void printn(uint64 n, uint64 base) {
    static char digits[] = "0123456789ABCDEF";
    uint64 m;
    if ((m = n / base)) {
        printn(m, base);
    }
    putchar(digits[(n % base)]);
}

void kprint(char *str) {
    for (uint64 i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}

void _kprintf(char *fmt, uint8 argc, uintptr *argv) {
    char *s;
    char c;
    uint8 arg_index = 0;

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            putchar(c);
            continue;
        }

        c = *fmt++;

        if (arg_index >= argc) {
            kprint("Error: Not enough arguments provided for format string.\n");
            return;
        }

        switch (c) {
            case 'd':
                printn((uint64)argv[arg_index++], 10);
                break;
            case 'x':
            case 'p':
                kprint("0x");
                printn((uint64)argv[arg_index++], 16);
                break;
            case 'o':
                kprint("0o");
                printn((uint64)argv[arg_index++], 8);
                break;
            case 'c':
                putchar(*(char *)argv[arg_index++]);
                break;
            case 's':
                s = (char *)argv[arg_index++];
                if (s == NULL) {
                    kprint("(null)");
                } else {
                    kprint(s);
                }
                break;
            case 'X':
                printn((uint64)argv[arg_index++], 16);
                break;
            default:
                kprint("Unknown format ");
                putchar(c);
                return;
        }
    }
}
