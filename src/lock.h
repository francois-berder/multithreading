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

#ifndef LOCK_H
#define LOCK_H

#include <stdatomic.h>

struct lock_t {
    atomic_int_least8_t counter;
};

/**
 * @brief Acquire lock
 *
 * Note that it calls scheduler_yield if it could
 * not acquire the lock.
 *
 * @param[in] l
 */
void lock_acquire(struct lock_t *l);

/**
 * @brief Release lock
 *
 * @param[in] l
 */
void lock_release(struct lock_t *l);

#endif
