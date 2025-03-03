#include <x86/asm.h>

#define COM1_PORT 0x3f8  // COM1

int serial_init() {
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

uint8 serial_ready() { return inb(COM1_PORT + 5) & 0x20; }

void serial_putchar(char c) {
    while (serial_ready() == 0);
    outb(COM1_PORT, c);
}