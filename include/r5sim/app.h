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
 * Application stuff; e.g the command line args to the simulator, etc.
 * Useful for code that wants to look at these things.
 */

#ifndef __R5SIM_APP_H__
#define __R5SIM_APP_H__

struct r5sim_app_args {
	int         help;
	int         verbose;
	int         itrace;
	const char *bootrom;
	const char *disk_file;
	const char *script;
};

struct r5sim_app_args *
r5sim_app_get_args(void);

#endif
