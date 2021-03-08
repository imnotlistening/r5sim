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
 * Simple framework for describing IO devices.
 */

#include <r5sim/log.h>
#include <r5sim/iodev.h>

void r5sim_iodev_describe(struct r5sim_iodev *dev)
{
	r5sim_info("IO Device: %s\n", dev->name);
	r5sim_info("  Aperture info:\n");
	r5sim_info("    Base: 0x%x\n", dev->io_offset);
	r5sim_info("    Size: 0x%x\n", dev->io_size);
}
