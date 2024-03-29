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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#ifndef TASK_COUNT
#define TASK_COUNT      (8)
#endif

#define MAIN_TASK_ID    (0)

enum task_status_t {
    TASK_STOPPED,
    TASK_SCHEDULED,
    TASK_RUNNING,
};

/**
 * @brief Start scheduler
 *
 * Create main task and start it.
 */
void scheduler_start(void);

/**
 * @brief Yield current task
 */
void scheduler_yield(void);

/**
 * @Brief Create a task
 *
 * Note that main task is created in scheduler_start.
 * Note that the task is not scheduled.
 *
 * @param[in] id Must be less than TASK_COUNT
 * @param[in] entrypoint
 * @param[in] stack
 * @param[in] stack_length
 */
void task_create(unsigned int id, void (*entrypoint)(void), void *stack, uint32_t stack_size);

/**
 * @brief Add task to scheduled task list
 *
 * @param[in] id
 */
void task_schedule(unsigned int id);

/**
 * @param[in] id
 * @return Task status
 */
enum task_status_t task_get_status(unsigned int id);

#endif
