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
#include <stdint.h>

#define DUMMY_TASK_ID       (1)

static uint8_t dummy_task_stack[1024];

void dummy_task(void)
{
    while (1) {
        scheduler_yield();
    }
}

void main(void)
{
    task_create(DUMMY_TASK_ID, dummy_task, dummy_task_stack, 1024);

    while (1) {
        scheduler_yield();
    }
}
