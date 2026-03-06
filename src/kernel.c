// src/kernel.c

#include <stdint.h>
#include <stddef.h>

#include "tty.h"
#include "serial.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "io.h"
#include "pit.h"
#include "paging.h"
#include "multiboot.h"
#include "pmm.h"
#include "heap.h"
#include "kpage.h"
#include "task.h"

static void keyboard_handler(regs_t *r)
{
    (void)r;
    uint8_t sc = inb(0x60);
    serial_puts("KBD ");
    serial_put_hex8(sc);
    serial_putc('\n');
}

static void task1(void)
{
    uint32_t n = 0;
    for (;;)
    {
        if ((n++ % 200) == 0)
        {
            serial_puts("TASK1 RUNNING\n");
        }
        __asm__ volatile("hlt");
    }
}

void kmain(uint32_t magic, uint32_t mbi_addr)
{
    tty_clear();
    tty_puts("MyOS booted.\n");

    serial_init();
    serial_puts("Serial OK\n");

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        serial_puts("Bad multiboot magic\n");
        for (;;)
            __asm__ volatile("hlt");
    }

    multiboot_info_t *mbi = (multiboot_info_t *)mbi_addr;
    pmm_init(mbi);

    uint32_t f1 = pmm_alloc_frame();
    uint32_t f2 = pmm_alloc_frame();
    serial_puts("ALLOC ");
    serial_put_hex32(f1);
    serial_putc(' ');
    serial_put_hex32(f2);
    serial_putc('\n');

    gdt_install();
    serial_puts("GDT OK\n");

    idt_install();
    isr_irq_install();
    serial_puts("IDT/ISRs OK\n");

    pic_remap(0x20, 0x28);
    pic_set_masks(0xFC, 0xFF); // IRQ0 + IRQ1 enabled
    serial_puts("PIC OK (IRQ0+IRQ1 enabled)\n");

    register_interrupt_handler(0x21, keyboard_handler); // IRQ1 = int 33
    serial_puts("Keyboard handler OK\n");

    pit_init(100); // IRQ0 = int 32
    serial_puts("PIT OK (100Hz)\n");

    paging_init();

    heap_init(0x400000); // heap at 4MB (safe, avoids TEMP_VADDR region)
    void *a = kmalloc(64);
    void *b = kmalloc(4096);
    serial_puts("KMALLOC ");
    serial_put_hex32((uint32_t)a);
    serial_putc(' ');
    serial_put_hex32((uint32_t)b);
    serial_putc('\n');

    kpage_init(0x00800000);

    void *p1 = kpage_alloc();
    void *p2 = kpage_alloc();

    serial_puts("KPAGE_ALLOC ");
    serial_put_hex32((uint32_t)p1);
    serial_putc(' ');
    serial_put_hex32((uint32_t)p2);
    serial_putc('\n');

    kpage_free(p1);
    serial_puts("KPAGE_FREE ");
    serial_put_hex32((uint32_t)p1);
    serial_putc('\n');

    tasking_init();
    task_create(task1);
    tasking_enable();

    __asm__ volatile("sti");
    serial_puts("STI: interrupts enabled. Press keys in QEMU.\n");
    tty_puts("Interrupts enabled. Press keys (check terminal).\n");

    // Let ticks happen for a moment (busy wait)
    for (volatile uint32_t i = 0; i < 20000000; i++)
    {
    }

    // Now test the page fault
    // serial_puts("Triggering page fault...\n");
    // volatile uint32_t *bad = (uint32_t *)0xDEADBEEF;
    // *bad = 0x12345678;

    for (;;)
    {
        __asm__ volatile("hlt");
    }
}