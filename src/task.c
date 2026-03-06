#include <stdint.h>
#include "task.h"
#include "kpage.h"
#include "serial.h"

typedef struct
{
    regs_t *frame; // points to saved regs frame (GS at top)
} task_t;

static task_t tasks[2];
static int task_count = 0;
static int current = 0;
static int enabled = 0;
static uint32_t sched_ticks = 0;

static regs_t *build_initial_frame(void (*entry)(void), uint32_t stack_page)
{
    uint32_t stack_top = stack_page + 0x1000;
    uint32_t *sp = (uint32_t *)stack_top;

// Build a fake interrupt frame that matches isr_common_stub restore order:
// [gs fs es ds][edi..eax][int_no][err_code][eip][cs][eflags]
#define PUSH(x) (*--sp = (uint32_t)(x))

    PUSH(0x202);           // EFLAGS (IF=1)
    PUSH(0x08);            // CS (kernel code)
    PUSH((uint32_t)entry); // EIP
    PUSH(0);               // err_code
    PUSH(0x20);            // int_no (pretend timer interrupt)

    // popa block (reverse of pusha)
    PUSH(0); // eax
    PUSH(0); // ecx
    PUSH(0); // edx
    PUSH(0); // ebx
    PUSH(0); // esp (dummy)
    PUSH(0); // ebp
    PUSH(0); // esi
    PUSH(0); // edi

    // segment pops
    PUSH(0x10); // ds
    PUSH(0x10); // es
    PUSH(0x10); // fs
    PUSH(0x10); // gs

    return (regs_t *)sp;
}

void tasking_init(void)
{
    // task0 = current kernel (main) task, frame will be filled on first schedule
    task_count = 1;
    tasks[0].frame = 0;
    current = 0;
    enabled = 0;
    sched_ticks = 0;

    serial_puts("TASKING INIT\n");
}

void task_create(void (*entry)(void))
{
    if (task_count >= 2)
    {
        serial_puts("task_create: max tasks\n");
        return;
    }

    void *page = kpage_alloc(); // 1 page stack
    if (!page)
    {
        serial_puts("task_create: no stack\n");
        return;
    }

    tasks[task_count].frame = build_initial_frame(entry, (uint32_t)page);
    task_count++;

    serial_puts("TASK CREATED\n");
}

void tasking_enable(void)
{
    enabled = 1;
    serial_puts("TASKING ENABLED\n");
}

regs_t *task_schedule(regs_t *cur_frame)
{
    if (!enabled || task_count < 2)
        return cur_frame;

    // switch every 50 timer interrupts (reduce flicker/spam)
    sched_ticks++;
    if ((sched_ticks % 50) != 0)
        return cur_frame;

    // save current
    tasks[current].frame = cur_frame;

    // next
    current = (current + 1) % task_count;
    return tasks[current].frame ? tasks[current].frame : cur_frame;
}