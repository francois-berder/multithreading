/*
 * Copyright (C) 2019  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of multithreading.
 *
 * multithreading is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * multithreading is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with multithreading.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scheduler.h"
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

/* Task flags */
#define TASK_ACTIVE    (1)

#define THUMB_STATE     (1U << 24)

#define EXC_RETURN      (0xFFFFFFFD)

#ifdef __FPU_PRESENT
#define MIN_STACK_LENGTH    (128)
#else
#define MIN_STACK_LENGTH    (64)
#endif
#define MAIN_STACK_LENGTH   (1024)

struct task_t {
    uint32_t stack_pointer;
#ifdef __FPU_PRESENT
    uint32_t exception_code;
#endif
    enum task_status_t status;
    struct task_t *next;
};

static struct task_t tasks[TASK_COUNT];

/*
 * We need to force GCC to give these variables an address,
 * and not try to optimize too much.
 */
static __attribute__((used)) struct task_t *current_task;
static __attribute__((used)) struct task_t *next_task;

static struct task_t *scheduled_tasks;

static uint8_t __attribute__((aligned(64))) main_stack[MAIN_STACK_LENGTH];

void main(void);

void __attribute__((naked)) svcall_handler(void)
{
    __asm__ volatile(
        ".thumb_func\n"

        "cpsid i\n"

        /* Read stack pointer of current_task */
        "ldr r1, =current_task\n"
        "ldr r1, [r1]\n"
        "ldr r0, [r1]\n"

        /* Load context of current_task */
        "ldmfd r0!, {r4-r11}\n"
        "msr psp, r0\n"

#ifdef __FPU_PRESENT
        "ldr lr, [r1, #4]\n"
#else
        "mov lr, 0xFFFFFFFD\n"
#endif

        "cpsie i\n"
        "bx lr\n"
    );
}

void __attribute__((naked)) pendsv_handler(void)
{
    __asm__ volatile(
        ".thumb_func\n"

        "cpsid i\n"

        /* Load current_task and next_task */
        "ldr r0, =current_task\n"
        "ldr r2, =next_task\n"

        /* Check if switching to same task */
        "cmp r0, r2\n"
        "beq context_switch_end\n"

        /* Dereference current_task and next_task */
        "ldr r1, [r0]\n"
        "ldr r3, [r2]\n"

        /* Save context of current_task */
        "mrs r12, psp\n"

#ifdef __FPU_PRESENT
        "tst lr, #0x00000010\n"
        "it eq\n"
        "vstmdbeq r12!, {s16-s31}\n"
        "str lr, [r1, #4]\n"   /* Save exception code */
#endif
        "stmfd r12!, {r4-r11}\n"
        "str r12, [r1]\n"

        /* Load context of next_task */
        "ldr r1, [r3]\n"
        "ldmfd r1!, {r4-r11}\n"
#ifdef __FPU_PRESENT
        "ldr lr, [r3, #4]\n"    /* Load exception code */
        "tst lr, #0x00000010\n"
        "it eq\n"
        "vldmiaeq r1!, {s16-s31}\n"
#endif
        "msr psp, r1\n"

        /* Set current_task to next_task */
        "str r3, [r0]\n"

        "context_switch_end:\n"

        /* Set next_task to NULL */
        "mov r1, #0\n"
        "str r1, [r2]\n"

        "cpsie i\n"
        "bx lr\n"
    );
}

/* Force GCC not to generate code for the stack */
static noreturn void stop_task(void)
{
    /*
     * We reach this function if the task returns from the entry point.
     * In other words, the task finished its job.
     */
    scheduler_yield();
    __builtin_unreachable();
}

noreturn void scheduler_start(void)
{
    NVIC_SetPriority(PendSV_IRQn, 255);

    /*
     * 1. Create main and idle tasks
     * 2. Select main task
     * 3. Trigger SVC interrupt to start main task
     */
    task_create(MAIN_TASK_ID, main, main_stack, MAIN_STACK_LENGTH);

    current_task = &tasks[MAIN_TASK_ID];
    current_task->status = TASK_RUNNING;
    next_task = NULL;
    __asm__ volatile ("cpsie i" : : : "memory");
    __asm__ volatile ("svc 0");

    __builtin_unreachable();
}

void scheduler_yield(void)
{
scheduler_yield_start:

    __asm__ volatile ("cpsid i" : : : "memory");

    current_task->status = TASK_STOPPED;

    if (scheduled_tasks) {
        next_task = scheduled_tasks;
        scheduled_tasks = scheduled_tasks->next;
        next_task->next = NULL;
    }

    if (next_task) {
        next_task->status = TASK_RUNNING;
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        __asm__ volatile ("cpsie i" : : : "memory");
    }  else {
        __asm__ volatile ("wfi" ::: "memory");
        __asm__ volatile ("cpsie i" : : : "memory");
        goto scheduler_yield_start;
    }
}

void task_create(unsigned int id, void (*entrypoint)(void), void *stack, uint32_t stack_size)
{
    uint32_t *sp;

    sp = (uint32_t *)((uint32_t)stack + stack_size);

    /*
     * xPSR, PC, LR, R12, R3, R2, R1, R0 are restored by the
     * hardware upon leaving exception mode.
     */
    *--sp = ((uint32_t)entrypoint & 0x1) ? THUMB_STATE : 0;
    *--sp = (uint32_t)entrypoint;
    *--sp = (uint32_t)stop_task;
    *--sp = 12;
    *--sp = 3;
    *--sp = 2;
    *--sp = 1;
    *--sp = 0;

    /* R4-R11 need to be manually restored */
    *--sp = 11;
    *--sp = 10;
    *--sp = 9;
    *--sp = 8;
    *--sp = 7;
    *--sp = 6;
    *--sp = 5;
    *--sp = 4;

    tasks[id].stack_pointer = (uint32_t)sp;
#ifdef __FPU_PRESENT
    tasks[id].exception_code = EXC_RETURN;
#endif
    tasks[id].status = TASK_STOPPED;
}

void task_schedule(unsigned int id)
{
    if (scheduled_tasks) {
        struct task_t *tail;

        /*
         * Append to the end of the list. We assume
         * that the scheduled list will never be very long.
         */
        tail = scheduled_tasks;
        while (tail->next)
            tail = tail->next;

        tail->next = &tasks[id];
    } else {
        scheduled_tasks = &tasks[id];
    }

    tasks[id].status = TASK_SCHEDULED;
    tasks[id].next = NULL;
}

enum task_status_t task_get_status(unsigned int id)
{
    return tasks[id].status;
}
