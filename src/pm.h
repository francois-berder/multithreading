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

#ifndef PM_H
#define PM_H

/**
 * @brief Enter low power mode
 *
 * By default, it executes WFI instruction.
 * The function can be redefined.
 */
void pm_enter(void);

#endif
