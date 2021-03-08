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
 * Simple virtual devices that we can use for default machines.
 */

#ifndef __R5SIM_VDEVS_H__
#define __R5SIM_VDEVS_H__

#include <r5sim/env.h>

struct r5sim_iodev;
struct r5sim_machine;

struct r5sim_iodev *r5sim_vuart_load_new(
	struct r5sim_machine *mach,
	u32 io_offs);
struct r5sim_iodev *r5sim_vsys_load_new(
	struct r5sim_machine *mach,
	u32 io_offs);
struct r5sim_iodev *r5sim_vdisk_load_new(
	struct r5sim_machine *mach,
	u32 io_offs, const char *path);

#endif
