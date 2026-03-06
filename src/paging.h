#pragma once
#include <stdint.h>

void paging_init(void);

uint32_t read_cr2(void);

// map/unmap
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
uint32_t paging_alloc_page(uint32_t virt, uint32_t flags);

void paging_unmap_page(uint32_t virt);
uint32_t paging_get_phys(uint32_t virt);