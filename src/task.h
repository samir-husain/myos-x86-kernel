#pragma once
#include <stdint.h>
#include "interrupts.h"

void tasking_init(void);
void task_create(void (*entry)(void));     // create 1 extra task (task1)
void tasking_enable(void);

// called from timer IRQ path
regs_t* task_schedule(regs_t* cur);