#include <boot/efi.h>

void puts(uint16 *s) { ST->ConOut->OutputString(ST->ConOut, s); }

void printn(uint64 n, uint64 base) {
    static uint16 digits[] = L"0123456789ABCDEF";
    static uint16 buf[2] = L"0";
    uint64 m;
    if ((m = n / base)) {
        printn(m, base);
    }
    buf[0] = digits[(n % base)];
    puts(buf);
}

void put_param(uint16 *s, uint64 n) {
    puts(s);
    puts(L": 0x");
    printn(n, 16);
    puts(L"\r\n");
}

void warn(uint64 status, uint16 *message) {
    if (status) {
        puts(message);
        puts(L": ");
        printn(status, 16);
        puts(L"\r\n");
    }
}

void assert(uint64 status, uint16 *message) {
    warn(status, message);
    if (status) {
        while (1);
    }
}

void panic(uint16 *message) {
    puts(message);
    while (1);
}