/*
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
 *   1. Place an address in the VDISK_DRAM_ADDR and VDISK_PAGE_START
 *      registers.
 *   2. Specify an op to execute: COPY_TO_DISK or COPY_TO_DRAM.
 *   3. Specify a number of pages to copy.
 *   4. Write to VDISK_EXEC. Make sure VDISK_BUSY is not set.
 *   5. Poll/check that VDISK_BUSY is 0.
 *
 */
#define VDISK_PRESENT		0x0
#define VDISK_PAGE_SIZE		0x4

#define VDISK_EXEC		0x10
#define VDISK_OP		0x14
#define VDISK_OP_COPY_TO_DRAM	0x1
#define VDISK_OP_COPY_TO_DISK	0x2
#define VDISK_DRAM_ADDR		0x18
#define VDISK_PAGE_START	0x1c
#define VDISK_PAGES		0x24

#define VDISK_SIZE_LO		0x30
#define VDISK_SIZE_HI		0x34

#define VDISK_MAX_REG		0x38

#endif
