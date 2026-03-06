#include <stdint.h>
#include "serial.h"
#include "io.h"

#define COM1 0x3F8

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static int serial_ready(void) {
    return (inb(COM1 + 5) & 0x20) != 0;
}

void serial_putc(char c) {
    while (!serial_ready()) {}
    outb(COM1, (uint8_t)c);
}

void serial_puts(const char* s) {
    for (uint32_t i = 0; s[i]; i++) serial_putc(s[i]);
}

static char hex_digit(uint8_t x) {
    x &= 0xF;
    return (x < 10) ? ('0' + x) : ('A' + (x - 10));
}

void serial_put_hex8(uint8_t v) {
    serial_putc(hex_digit(v >> 4));
    serial_putc(hex_digit(v));
}

void serial_put_hex32(uint32_t v) {
    serial_puts("0x");
    serial_putc(hex_digit((v >> 28) & 0xF));
    serial_putc(hex_digit((v >> 24) & 0xF));
    serial_putc(hex_digit((v >> 20) & 0xF));
    serial_putc(hex_digit((v >> 16) & 0xF));
    serial_putc(hex_digit((v >> 12) & 0xF));
    serial_putc(hex_digit((v >> 8) & 0xF));
    serial_putc(hex_digit((v >> 4) & 0xF));
    serial_putc(hex_digit(v & 0xF));
}