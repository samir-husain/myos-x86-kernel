#include <stdint.h>
#include <stddef.h>
#include "heap.h"
#include "paging.h"
#include "serial.h"

#define PAGE_SIZE 0x1000u

static uint32_t heap_cur = 0;
static uint32_t heap_mapped_end = 0;

static inline uint32_t align_up(uint32_t x, uint32_t a)
{
    return (x + (a - 1)) & ~(a - 1);
}

void heap_init(uint32_t heap_start)
{
    heap_cur = heap_start;
    heap_mapped_end = heap_start;

    // map first page
    if (!paging_alloc_page(heap_mapped_end, 3u))
    {
        serial_puts("heap_init: no frames\n");
        for (;;)
            __asm__ volatile("hlt");
    }
    heap_mapped_end += PAGE_SIZE;

    serial_puts("HEAP OK start=");
    serial_put_hex32(heap_start);
    serial_putc('\n');
}

void *kmalloc(size_t size)
{
    if (size == 0)
        return 0;

    uint32_t s = (uint32_t)size;
    s = align_up(s, 16);

    uint32_t start = heap_cur;
    uint32_t end = heap_cur + s;

    // map pages as needed
    while (end > heap_mapped_end)
    {
        if (!paging_alloc_page(heap_mapped_end, 3u))
        {
            serial_puts("kmalloc: OOM\n");
            return 0;
        }
        heap_mapped_end += PAGE_SIZE;
    }

    heap_cur = end;
    return (void *)start;
}