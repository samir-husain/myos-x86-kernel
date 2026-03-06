#include <stdint.h>
#include "gdt.h"

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

extern void gdt_flush(uint32_t gdt_ptr_addr);

static gdt_entry_t gdt[3];
static gdt_ptr_t gp;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low  = (base & 0xFFFF);
    gdt[num].base_mid  = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].gran      = (limit >> 16) & 0x0F;

    gdt[num].gran     |= (gran & 0xF0);
    gdt[num].access    = access;
}

void gdt_install(void) {
    gp.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // data

    gdt_flush((uint32_t)&gp);
}