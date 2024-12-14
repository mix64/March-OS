#include <serial.h>
#include <x86/asm.h>

#define COM1_PORT 0x3f8  // COM1

int init_serial() {
    outb(COM1_PORT + 1, 0x00);  // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);  //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);  // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2,
         0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set
    outb(COM1_PORT + 4, 0x1E);  // Set in loopback mode, test the serial chip
    outb(COM1_PORT + 0, 0xAE);  // Test serial chip (send byte 0xAE and check if
                                // serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(COM1_PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1_PORT + 4, 0x0F);
    return 0;
}

int is_transmit_empty() { return inb(COM1_PORT + 5) & 0x20; }

void putchar(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1_PORT, c);
}

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
            default:
                kprint("Unknown format ");
                putchar(c);
                return;
        }
    }
}
