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
#include <stdnoreturn.h>
#include <stdint.h>

extern uint32_t _end_stack;
extern uint32_t _end_text;
extern uint32_t _start_data;
extern uint32_t _end_data;
extern uint32_t _start_bss;
extern uint32_t _end_bss;

noreturn void reset_handler(void)
{
    register uint32_t *src, *dst;

    /* Copy data section from flash to RAM */
    src = &_end_text;
    dst = &_start_data;
    while (dst < &_end_data)
        *dst++ = *src++;

    /* Clear the bss section, assumes .bss goes directly after .data */
    dst = &_start_bss;
    while (dst < &_end_bss)
        *dst++ = 0;

#ifdef __FPU_PRESENT
    SCB->CPACR |= 0x00f00000;
#endif

    scheduler_start();
    __builtin_unreachable();
}

void default_handler(void)
{
    __asm__ volatile ("cpsid i" ::: "memory");
    while (1);
}

void nmi_handler(void) __attribute__((weak, alias("default_handler")));
void hardfault_handler(void) __attribute__((weak, alias("default_handler")));
void svcall_handler(void) __attribute__((weak, alias("default_handler")));
void pendsv_handler(void) __attribute__((weak, alias("default_handler")));
void systick_handler(void) __attribute__((weak, alias("default_handler")));

void * const core_vector_table[16] __attribute__ ((section(".cortex_vectors"))) = {
    [0] = &_end_stack,
    [1] = reset_handler,
    [2] = nmi_handler,
    [3] = hardfault_handler,
    [11] = svcall_handler,
    [14] = pendsv_handler,
    [15] = systick_handler,
};
