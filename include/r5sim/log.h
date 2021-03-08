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
 * Logging routines for printing simulation info to the terminal.
 */

#ifndef __R5SIM_LOG_H__
#define __R5SIM_LOG_H__

#include <stdio.h>
#include <stdarg.h>

enum r5sim_log_level {
	ERR = 0,
	WARN,
	INFO,
	DEBUG,
	DEBUG_V,
	DEBUG_VV,
};

#define r5sim_err(fmt, args...)						\
	__r5sim_log_print(ERR, __FILE__, __LINE__, fmt, ##args)

#define r5sim_warn(fmt, args...)					\
	__r5sim_log_print(WARN, __FILE__, __LINE__, fmt, ##args)

#define r5sim_info(fmt, args...)					\
	__r5sim_log_print(INFO, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg(fmt, args...)						\
	__r5sim_log_print(DEBUG, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg_v(fmt, args...)					\
	__r5sim_log_print(DEBUG_V, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg_vv(fmt, args...)					\
	__r5sim_log_print(DEBUG_VV, __FILE__, __LINE__, fmt, ##args)

__attribute__((format (printf, 4, 5)))
void __r5sim_log_print(enum r5sim_log_level lvl,
		       const char *file,
		       int line,
		       const char *fmt, ...);

/*
 * Separate from the rest of the logging - itrace'ing is _very_ verbose.
 * It also needs to be quick; since this is present in the instruction
 * execution path and memory load paths, determining whether to itrace needs
 * to be quick.
 *
 * Here we have a single branch instruction; theoretically it could perhaps
 * be better if we did some dynamic code updating or some such but that's
 * complicated. That potentially requires instruction cache updates and..
 * Yeah, I dunno - surely modern branch predictors will make this execute
 * pretty fast 99.999... % of the time.
 */
#define r5sim_itrace(core, fmt, args...)			\
	do {							\
		if ((core)->itrace)				\
			__r5sim_itrace_print(fmt, ##args);	\
	} while (0)

__attribute__((format (printf, 1, 2)))
static inline void __r5sim_itrace_print(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void) vprintf(fmt, args);
	va_end(args);
}

__attribute__((format (printf, 1, 2)))
void __r5sim_itrace_print(const char *fmt, ...);

#endif
