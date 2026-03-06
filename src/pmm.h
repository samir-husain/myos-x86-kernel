#pragma once
#include <stdint.h>
#include "multiboot.h"

void pmm_init(multiboot_info_t* mbi);
uint32_t pmm_alloc_frame(void);     // returns physical address, 0 if fail
void pmm_free_frame(uint32_t phys);