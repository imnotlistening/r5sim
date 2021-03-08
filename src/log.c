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
 * Super trivial logging!
 */

#include <stdio.h>
#include <stdarg.h>

#include <r5sim/log.h>
#include <r5sim/app.h>

#define R5SIM_FMT	"[%s] %15s:%-4d | %s"
#define BUF_LIMIT	256

static const char *level_messages[4] = {
	"E",
	"W",
	"I",
	"D",
};

static const char *level_to_str(enum r5sim_log_level lvl)
{
	if (lvl > 3)
		lvl = 3;

	return level_messages[lvl];
}

__attribute__((format (printf, 4, 5)))
void __r5sim_log_print(enum r5sim_log_level lvl,
		       const char *file,
		       int line,
		       const char *fmt, ...)
{
	va_list args;
	char buf[BUF_LIMIT];
	struct r5sim_app_args *app = r5sim_app_get_args();

	if (lvl > app->verbose)
		return;

	va_start(args, fmt);
	(void) vsnprintf(buf, BUF_LIMIT, fmt, args);
	va_end(args);

	fprintf(lvl > WARN ? stdout : stderr,
		R5SIM_FMT,
		level_to_str(lvl),
		file, line, buf);
}
