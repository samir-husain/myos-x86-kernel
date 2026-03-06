#pragma once
#include <stdint.h>

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} regs_t;

typedef void (*isr_t)(regs_t*);

void isr_irq_install(void);
void register_interrupt_handler(uint8_t n, isr_t handler);

void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_set_masks(uint8_t master_mask, uint8_t slave_mask);