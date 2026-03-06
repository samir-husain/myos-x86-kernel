#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init(uint32_t heap_start);
void* kmalloc(size_t size);