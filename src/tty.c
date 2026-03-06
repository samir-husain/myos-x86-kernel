#include <stdint.h>
#include <stddef.h>
#include "tty.h"

static volatile uint16_t* const VGA = (uint16_t*)0xB8000;
static const size_t VGA_W = 80;
static const size_t VGA_H = 25;

static size_t row = 0, col = 0;
static uint8_t color = 0x0F;

static inline uint16_t vga_entry(char c, uint8_t colr) {
    return (uint16_t)c | ((uint16_t)colr << 8);
}

void tty_clear(void) {
    for (size_t y = 0; y < VGA_H; y++)
        for (size_t x = 0; x < VGA_W; x++)
            VGA[y * VGA_W + x] = vga_entry(' ', color);
    row = 0;
    col = 0;
}

void tty_putc(char c) {
    if (c == '\n') {
        col = 0;
        if (++row >= VGA_H) row = 0;
        return;
    }
    VGA[row * VGA_W + col] = vga_entry(c, color);
    if (++col >= VGA_W) {
        col = 0;
        if (++row >= VGA_H) row = 0;
    }
}

void tty_puts(const char* s) {
    for (size_t i = 0; s[i]; i++) tty_putc(s[i]);
}