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

#include "pm.h"
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
    uint32_t flags;
};

static struct task_t tasks[TASK_COUNT];

/*
 * We need to force GCC to give these variables an address,
 * and not try to optimize too much.
 */
static __attribute__((used)) struct task_t *current_task;
static __attribute__((used)) struct task_t *next_task;

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
    current_task->flags &= ~TASK_ACTIVE;
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
    next_task = NULL;
    __asm__ volatile ("cpsie i" : : : "memory");
    __asm__ volatile ("svc 0");

    __builtin_unreachable();
}

void scheduler_yield(void)
{
    int i;
    int current_task_index;


    /* Select next task to run */
    current_task_index = ((uint32_t)current_task - (uint32_t)tasks) / sizeof(struct task_t);
    /* next_task was cleared in the last context switch */
    while (!next_task) {
        __asm__ volatile ("cpsid i" : : : "memory");
        for (i = 0; i < TASK_COUNT; ++i) {
            int index = current_task_index + i + 1;
            if (index >= TASK_COUNT)
                index -= TASK_COUNT;

            if (tasks[index].flags & TASK_ACTIVE) {
                next_task = &tasks[index];
                break;
            }
        }
        if (!next_task) {
            pm_enter();
            __asm__ volatile ("cpsie i" : : : "memory");
        }
    }

    __asm__ volatile ("cpsie i" : : : "memory");
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
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
    tasks[id].flags = TASK_ACTIVE;
}

void task_resume(unsigned int id)
{
    tasks[id].flags |= TASK_ACTIVE;
}

void task_pause(unsigned int id)
{
    tasks[id].flags &= ~TASK_ACTIVE;
    if (current_task == &tasks[id])
        scheduler_yield();
}
