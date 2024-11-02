#include "efi.h"

void puts(uint16 *s) {
    ST->ConOut->OutputString(ST->ConOut, s);
}

void put_hex(uint64 n) {
    uint16 hex[] = L"0123456789ABCDEF";
    uint16 buf[17];
    buf[16] = 0;
    for (int i = 0; i < 16; i++) {
        buf[15 - i] = hex[n & 0xF];
        n >>= 4;
    }
    puts(L"0x");
    puts(buf);
}

void put_param(uint16 *s, uint64 n) {
    puts(s);
    puts(L": ");
    put_hex(n);
    puts(L"\r\n");
}

void put_warn(uint64 status, uint16 *message)
{
    if (status) {
        puts(message);
        puts(L":");
        put_hex(status);
        puts(L"\r\n");
    }
}

void assert(uint64 status, uint16 *message)
{
    put_warn(status, message);
    if (status) {
        while (1);
	}
}