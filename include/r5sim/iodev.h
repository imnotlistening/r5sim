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
 * An IO device model.
 */

#ifndef __R5SIM_IODEV_H__
#define __R5SIM_IODEV_H__

#include <r5sim/env.h>
#include <r5sim/list.h>

struct r5sim_machine;

struct r5sim_iodev {
	const char           *name;

	/*
	 * Pointer back to the machine for doing "DMA".
	 */
	struct r5sim_machine *mach;

	/*
	 * Base IO address offset. This is offset from machine.iomem_base.
	 */
	u32              io_offset;
	u32              io_size;

	/*
	 * IO accessor functions; these are 0 based from io_offset.
	 */
	u32            (*readl)(struct r5sim_iodev *dev, u32 offs);
	void           (*writel)(struct r5sim_iodev *dev,
				 u32 offs, u32 val);

	/*
	 * List entry for when this device is attached to a machine.
	 */
	struct list_head      mach_node;

	/*
	 * Private data for implementing the device.
	 */
	void                 *priv;
};

void r5sim_iodev_describe(struct r5sim_iodev *dev);

#endif
