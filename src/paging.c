#include <stdint.h>
#include "paging.h"
#include "interrupts.h"
#include "serial.h"
#include "pmm.h"

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

#define TEMP_VADDR 0x003FF000u // last page in first 4MB

static inline void write_cr3(uint32_t val)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(val));
}
static inline uint32_t read_cr0(void)
{
    uint32_t val;
    __asm__ volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}
static inline void write_cr0(uint32_t val)
{
    __asm__ volatile("mov %0, %%cr0" : : "r"(val));
}
uint32_t read_cr2(void)
{
    uint32_t val;
    __asm__ volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}
static inline void invlpg(void *m)
{
    __asm__ volatile("invlpg (%0)" : : "r"(m) : "memory");
}

static void mem_zero32(uint32_t *p, uint32_t dwords)
{
    while (dwords--)
        *p++ = 0;
}

// temp-map any physical 4KB page at TEMP_VADDR using first_page_table[1023]
static uint32_t *temp_map(uint32_t phys)
{
    first_page_table[1023] = (phys & 0xFFFFF000u) | 3u; // present + rw
    invlpg((void *)TEMP_VADDR);
    return (uint32_t *)TEMP_VADDR;
}

static void page_fault_handler(regs_t *r)
{
    uint32_t fault_addr = read_cr2();
    serial_puts("PAGE FAULT addr=");
    serial_put_hex32(fault_addr);
    serial_puts(" err=");
    serial_put_hex32(r->err_code);
    serial_putc('\n');
    for (;;)
        __asm__ volatile("hlt");
}

void paging_init(void)
{
    // identity map first 4MB
    for (int i = 0; i < 1024; i++)
    {
        first_page_table[i] = (i * 0x1000u) | 3u; // present + rw
    }

    // reserve TEMP slot (we will overwrite it dynamically)
    // keep it present initially to avoid surprises
    first_page_table[1023] = (1023u * 0x1000u) | 3u;

    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0x00000002u; // rw, not present
    }

    page_directory[0] = ((uint32_t)first_page_table) | 3u;

    register_interrupt_handler(14, page_fault_handler);

    write_cr3((uint32_t)page_directory);
    uint32_t cr0 = read_cr0();
    cr0 |= 0x80000000u; // PG
    write_cr0(cr0);

    serial_puts("PAGING ON (identity 0-4MB)\n");
}

void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FFu;

    uint32_t pde = page_directory[dir_idx];

    // if page table missing, allocate one
    if ((pde & 1u) == 0)
    {
        uint32_t pt_phys = pmm_alloc_frame();
        if (!pt_phys)
        {
            serial_puts("OOM: no frames for PT\n");
            for (;;)
                __asm__ volatile("hlt");
        }

        uint32_t *pt_tmp = temp_map(pt_phys);
        mem_zero32(pt_tmp, 1024);

        page_directory[dir_idx] = (pt_phys & 0xFFFFF000u) | 3u; // present + rw
        pde = page_directory[dir_idx];
    }

    uint32_t pt_phys = pde & 0xFFFFF000u;

    // access page table
    uint32_t *pt;
    if (dir_idx == 0)
    {
        pt = first_page_table;
    }
    else
    {
        pt = temp_map(pt_phys);
    }

    pt[tbl_idx] = (phys & 0xFFFFF000u) | (flags & 0xFFFu);
    invlpg((void *)virt);
}

uint32_t paging_alloc_page(uint32_t virt, uint32_t flags)
{
    uint32_t phys = pmm_alloc_frame();
    if (!phys)
        return 0;
    paging_map_page(virt, phys, flags);
    return phys;
}

uint32_t paging_get_phys(uint32_t virt)
{
    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FFu;

    uint32_t pde = page_directory[dir_idx];
    if ((pde & 1u) == 0)
        return 0;

    uint32_t pt_phys = pde & 0xFFFFF000u;

    uint32_t *pt;
    if (dir_idx == 0)
    {
        pt = first_page_table;
    }
    else
    {
        pt = temp_map(pt_phys);
    }

    uint32_t pte = pt[tbl_idx];
    if ((pte & 1u) == 0)
        return 0;

    return (pte & 0xFFFFF000u) | (virt & 0xFFFu);
}

void paging_unmap_page(uint32_t virt)
{
    // do not allow unmapping our temp mapping slot
    if ((virt & 0xFFFFF000u) == TEMP_VADDR)
    {
        serial_puts("paging_unmap_page: blocked TEMP_VADDR\n");
        return;
    }

    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FFu;

    uint32_t pde = page_directory[dir_idx];
    if ((pde & 1u) == 0)
        return;

    uint32_t pt_phys = pde & 0xFFFFF000u;

    uint32_t *pt;
    if (dir_idx == 0)
    {
        pt = first_page_table;
    }
    else
    {
        pt = temp_map(pt_phys);
    }

    uint32_t pte = pt[tbl_idx];
    if ((pte & 1u) == 0)
        return;

    uint32_t phys = pte & 0xFFFFF000u;

    pt[tbl_idx] = 0;
    invlpg((void *)virt);

    pmm_free_frame(phys);
}