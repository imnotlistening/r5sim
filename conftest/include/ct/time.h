/* Copyright 2021, Alex Waterman <imnotlistening@gmail.com>
 *
 * This file is part of r5sim.
 *
 * r5sim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * r5sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with r5sim.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Define a few functions to help out with keeping track of
 * time and computing core perf.
 */

#ifndef __CT_TIME_H__
#define __CT_TIME_H__

#include <ct/types.h>

struct ct_time {
	u32 lo;
	u32 hi;
};

u32  ct_compute_perf(void);
void ct_time_diff(struct ct_time *dst, struct ct_time *a, struct ct_time *b);
void ct_ptime(struct ct_time *t, const char *str);

#endif
