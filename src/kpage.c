#include <stdint.h>
#include "kpage.h"
#include "paging.h"
#include "serial.h"

#define PAGE_SIZE 0x1000u

static uint32_t kpage_cur = 0;

void kpage_init(uint32_t base)
{
    kpage_cur = (base + 0xFFFu) & ~0xFFFu;
    serial_puts("KPAGE OK base=");
    serial_put_hex32(kpage_cur);
    serial_putc('\n');
}

void *kpage_alloc(void)
{
    uint32_t v = kpage_cur;
    if (!paging_alloc_page(v, 3u))
    {
        serial_puts("kpage_alloc: OOM\n");
        return 0;
    }
    kpage_cur += PAGE_SIZE;
    return (void *)v;
}

void kpage_free(void *p)
{
    if (!p)
        return;
    paging_unmap_page((uint32_t)p);
}