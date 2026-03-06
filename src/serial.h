#pragma once
#include <stdint.h>

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* s);
void serial_put_hex8(uint8_t v);
void serial_put_hex32(uint32_t v);