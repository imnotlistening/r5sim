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
 * Various environment functions that should be generally useful.
 */

#ifndef __R5SIM_ENV_H__
#define __R5SIM_ENV_H__

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <execinfo.h>

static inline void __r5sim_backtrace(void)
{
	int frames, i;
	char **symbols;
	void *functions[32];

	printf("Ahh! Crash! Backtrace:\n");

	frames = backtrace((void **)functions, 32);
	symbols = backtrace_symbols((void **)functions, frames);

	for (i = 0; i < frames; i++)
		printf("  #%-2d %s()\n", i, symbols[i]);
}

#define r5sim_assert(cond)				\
	do {						\
		if (!(cond)) {				\
			__r5sim_backtrace();		\
							\
			/* Actually assert now. */	\
			assert(cond);			\
		}					\
	} while (0)

/*
 * Convenience macros for sizes.
 */
#define GB(x) ((x) << 30)
#define MB(x) ((x) << 20)
#define KB(x) ((x) << 10)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

#endif
