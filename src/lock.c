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

#include "lock.h"
#include "scheduler.h"
#include <stdatomic.h>

void lock_acquire(struct lock_t *l)
{
    int_least8_t val = 1;
    while (!atomic_compare_exchange_weak(&l->counter, &val, 0))
        scheduler_yield();
}

void lock_release(struct lock_t *l)
{
    atomic_store(&l->counter, 0);
}
