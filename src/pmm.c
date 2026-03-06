#include <stdint.h>
#include "pmm.h"
#include "serial.h"

extern uint8_t _kernel_end;

static uint8_t *bitmap = 0;
static uint32_t total_frames = 0;
static uint32_t bitmap_bytes = 0;

static void mem_set(void *dst, uint8_t v, uint32_t n)
{
    uint8_t *p = (uint8_t *)dst;
    while (n--)
        *p++ = v;
}

static inline void bm_set(uint32_t i) { bitmap[i >> 3] |= (1u << (i & 7)); }    // used
static inline void bm_clear(uint32_t i) { bitmap[i >> 3] &= ~(1u << (i & 7)); } // free
static inline uint8_t bm_test(uint32_t i) { return (bitmap[i >> 3] >> (i & 7)) & 1u; }

static void mark_region_free(uint64_t base, uint64_t len)
{
    uint64_t start = base;
    uint64_t end = base + len;

    // align to 4K
    if (start & 0xFFF)
        start = (start + 0xFFF) & ~0xFFFULL;
    end &= ~0xFFFULL;

    for (uint64_t addr = start; addr < end; addr += 0x1000ULL)
    {
        uint64_t frame = addr >> 12;
        if (frame < total_frames)
            bm_clear((uint32_t)frame);
    }
}

void pmm_init(multiboot_info_t *mbi)
{
    if (!mbi || !(mbi->flags & MULTIBOOT_INFO_MEM_MAP))
    {
        serial_puts("PMM: no mmap\n");
        for (;;)
            __asm__ volatile("hlt");
    }

    // find max physical end from mmap
    uint64_t max_end = 0;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    for (uint32_t p = mbi->mmap_addr; p < mmap_end;)
    {
        multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t *)p;
        uint64_t end = e->addr + e->len;
        if (end > max_end)
            max_end = end;
        p += e->size + 4;
    }

    total_frames = (uint32_t)(max_end >> 12);
    bitmap_bytes = (total_frames + 7) / 8;

    // place bitmap right after kernel end (aligned)
    uintptr_t bm_start = ((uintptr_t)&_kernel_end + 0xFFF) & ~0xFFF;
    bitmap = (uint8_t *)bm_start;

    // default: all used
    mem_set(bitmap, 0xFF, bitmap_bytes);

    // mark all "available" regions as free (type==1)
    for (uint32_t p = mbi->mmap_addr; p < mmap_end;)
    {
        multiboot_mmap_entry_t *e = (multiboot_mmap_entry_t *)p;
        if (e->type == 1)
            mark_region_free(e->addr, e->len);
        p += e->size + 4;
    }

    // re-mark reserved frames: [0 .. bitmap end)
    uintptr_t bm_end = bm_start + bitmap_bytes;
    uint32_t reserved_frames = (uint32_t)((bm_end + 0xFFF) >> 12);
    for (uint32_t i = 0; i < reserved_frames; i++)
        bm_set(i);

    bm_set(0); // never allocate frame 0

    serial_puts("PMM OK frames=");
    serial_put_hex32(total_frames);
    serial_putc('\n');
}

uint32_t pmm_alloc_frame(void)
{
    for (uint32_t i = 0; i < total_frames; i++)
    {
        if (!bm_test(i))
        {
            bm_set(i);
            return i << 12;
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t phys)
{
    uint32_t i = phys >> 12;
    if (i < total_frames)
        bm_clear(i);
}