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
 * VDISK shared defines for the HW and for the "driver".
 */

#ifndef __R5SIM_HW_VDISK_H__
#define __R5SIM_HW_VDISK_H__

/*
 * Disk device "registers". These are the offsets divided by 4 (so that
 * we can easily access the dev_state array. This provides a model that
 * theoretically could someday be asynchronous to the rest of the
 * simulator. For now, since that support doesn't exist, part of this
 * sequence is not necessary (e.g polling on the op being done).
 *
 * The basic usage of this device is really quite simple.
 *
 *   1. Place an address in the VDISK_DRAM_ADDR and a page aligned
 *      offset in VDISK_PAGE_START.
 *   2. Specify a number of pages to copy.
 *   3. Specify an op to execute: COPY_TO_DISK or COPY_TO_DRAM.
 *   4. Make sure VDISK_BUSY is not set.
 *   4. Write to VDISK_EXEC.
 *   5. Poll/check that VDISK_BUSY is 0.
 *
 */
#define VDISK_PRESENT		0x0
#define VDISK_PAGE_SIZE		0x4
#define VDISK_SIZE_LO		0x8
#define VDISK_SIZE_HI		0xc

#define VDISK_DRAM_ADDR		0x10
#define VDISK_PAGE_START	0x14
#define VDISK_PAGES		0x18
#define VDISK_OP		0x1c
#define VDISK_OP_COPY_TO_DRAM	0x1
#define VDISK_OP_COPY_TO_DISK	0x2

#define VDISK_EXEC		0x20
#define VDISK_BUSY		0x24

#define VDISK_MAX_REG		0x30

#endif
