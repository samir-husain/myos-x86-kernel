#pragma once
#include <stdint.h>

void kpage_init(uint32_t base);
void* kpage_alloc(void);      // 1 page (4KB)
void  kpage_free(void* p);    // frees 1 page