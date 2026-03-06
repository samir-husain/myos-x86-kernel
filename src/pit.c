#include <stdint.h>
#include "pit.h"
#include "io.h"
#include "interrupts.h"
#include "serial.h"

static volatile uint32_t ticks = 0;

static void pit_handler(regs_t *r)
{
    (void)r;
    ticks++;

    // print every 100 ticks to avoid spam
    if ((ticks % 100) == 0)
    {
        serial_puts("TICK ");
        serial_put_hex32(ticks);
        serial_putc('\n');
    }
}

uint32_t pit_get_ticks(void)
{
    return ticks;
}

void pit_init(uint32_t hz)
{
    // PIT base frequency
    const uint32_t PIT_HZ = 1193182;
    if (hz == 0)
        hz = 100;

    uint32_t divisor = PIT_HZ / hz;
    if (divisor < 1)
        divisor = 1;
    if (divisor > 65535)
        divisor = 65535;

    // register handler for IRQ0 (int 32)
    register_interrupt_handler(0x20, pit_handler);

    // command: channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}