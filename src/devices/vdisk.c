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
 * Define a virtual disk like device that can be used for accessing a
 * permanent store of memory.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/iodev.h>
#include <r5sim/vdevs.h>
#include <r5sim/machine.h>

#include <r5sim/hw/vdisk.h>

#define vdisk_dbg r5sim_dbg_v

struct virt_disk_priv {
	int	 fd;
	void	*mmap;

	size_t	 size;

	u32 dev_state[VDISK_MAX_REG >> 2];
};

static const char *vdisk_reg_to_str(u32 reg)
{
	static const char *str_reg[] = {
		[VDISK_PRESENT]		= "VDISK_PRESENT",
		[VDISK_PAGE_SIZE]	= "VDISK_PAGE_SIZE",
		[VDISK_SIZE_LO]		= "VDISK_LO",
		[VDISK_SIZE_HI]		= "VDISK_HI",
		[VDISK_DRAM_ADDR]	= "VDISK_DRAM_ADDR",
		[VDISK_PAGE_START]	= "VDISK_START",
		[VDISK_PAGES]		= "VDISK_PAGES",
		[VDISK_OP]		= "VDISK_OP",
		[VDISK_EXEC]		= "VDISK_EXEC",
		[VDISK_BUSY]		= "VDISK_BUSY",
	};

	r5sim_assert(reg <= VDISK_MAX_REG);

	return str_reg[reg];
}

/*
 * Set a state register for the device.
 */
static void __vdisk_set_state(struct virt_disk_priv *disk, u32 __i, u32 val)
{
	u32 i = __i >> 2;

	r5sim_assert(i < VDISK_MAX_REG);

	disk->dev_state[i] = val;
}

static u32 __vdisk_read_state(struct virt_disk_priv *disk, u32 __i)
{
	u32 i = __i >> 2;

	r5sim_assert(i < VDISK_MAX_REG);

	return disk->dev_state[i];
}

static u32 virt_disk_readl(struct r5sim_iodev *iodev, u32 offs)
{
	vdisk_dbg("LOAD  @ %s\n",
		  vdisk_reg_to_str(offs));

	if (offs >= VDISK_MAX_REG)
		return 0x0;

	return __vdisk_read_state(iodev->priv, offs);
}

static void virt_disk_do_copy(struct virt_disk_priv *priv,
			      struct r5sim_machine *mach,
			      u32 op,
			      u32 dram_addr,
			      u32 page_start,
			      u32 pages)
{
	u32 disk_start = page_start * KB(4);
	u32 disk_bytes = pages * KB(4);

	if (disk_start >= priv->size) {
		vdisk_dbg("Read overrun!\n");
		return;
	}

	/*
	 * Only copy up to the size of the disk.
	 */
	if (disk_start + disk_bytes >= priv->size)
		disk_bytes = priv->size - disk_start - 1;

	if (op == VDISK_OP_COPY_TO_DRAM)
		memcpy(mach->memory + (dram_addr - mach->memory_base),
		       priv->mmap + disk_start,
		       disk_bytes);
	else
		memcpy(priv->mmap + disk_start,
		       mach->memory + (dram_addr - mach->memory_base),
		       disk_bytes);

}

static void virt_disk_exec_op(struct r5sim_iodev *iodev)
{
	struct virt_disk_priv *priv = iodev->priv;
	struct r5sim_machine *mach = iodev->mach;

	u32 dram_addr  = __vdisk_read_state(priv, VDISK_DRAM_ADDR);
	u32 page_start = __vdisk_read_state(priv, VDISK_PAGE_START);
	u32 pages      = __vdisk_read_state(priv, VDISK_PAGES);
	u32 op         = __vdisk_read_state(priv, VDISK_OP);

	if ((op & VDISK_OP_COPY_TO_DRAM) == 0 &&
	    (op & VDISK_OP_COPY_TO_DISK) == 0) {
		vdisk_dbg("noop.\n");
		return;
	}

	vdisk_dbg("op: %u\n", op);
	vdisk_dbg("  DRAM addr:   0x%08x\n", dram_addr);
	vdisk_dbg("  Page offset: %u\n", page_start);
	vdisk_dbg("  Pages:       %u\n", pages);

	if (op & VDISK_OP_COPY_TO_DRAM)
		virt_disk_do_copy(priv, mach, VDISK_OP_COPY_TO_DRAM,
				  dram_addr, page_start, pages);
	else
		virt_disk_do_copy(priv, mach, VDISK_OP_COPY_TO_DISK,
				  dram_addr, page_start, pages);
}

static void virt_disk_writel(struct r5sim_iodev *iodev,
			     u32 offs, u32 val)
{
	vdisk_dbg("STORE @ %-17s v=0x%08x\n",
		  vdisk_reg_to_str(offs), val);

	switch (offs) {
		/*
		 * Some registers are just direct write-throughs.
		 * These are easy: just store the value.
		 */
	case VDISK_DRAM_ADDR:
	case VDISK_PAGE_START:
	case VDISK_PAGES:
	case VDISK_OP:
		__vdisk_set_state(iodev->priv, offs, val);
		return;

		/*
		 * We also have the exec op register to execute a disk
		 * operation.
		 */
	case VDISK_EXEC:
		virt_disk_exec_op(iodev);
		return;

	default:
		vdisk_dbg("  Invalid WRITE.");
		return;
	}
}

static int vdisk_load(struct r5sim_machine *mach,
		      struct virt_disk_priv *disk,
		      const char *path)
{
	struct stat buf;

	memset(disk, 0, sizeof(*disk));

	disk->fd = open(path, O_RDWR);
	if (disk->fd < 0) {
		perror(path);
		r5sim_assert(!"Failed to open VDISK path");
	}

	if (fstat(disk->fd, &buf) < 0) {
		perror(path);
		r5sim_assert(!"Failed to stat!");
	}

	disk->size = buf.st_size;

	disk->mmap = mmap(NULL, disk->size, PROT_READ|PROT_WRITE,
			  MAP_SHARED, disk->fd, 0x0);
	if (disk->mmap == MAP_FAILED) {
		perror(path);
		r5sim_assert(!"Failed to mmap!");
	}

	/*
	 * Init some basic settings.
	 */
	__vdisk_set_state(disk, VDISK_PRESENT,   0x1);
	__vdisk_set_state(disk, VDISK_PAGE_SIZE, 4096);
	__vdisk_set_state(disk, VDISK_SIZE_LO,   disk->size & 0xFFFFFFFF);
	__vdisk_set_state(disk, VDISK_SIZE_HI,   disk->size >> 32);

	return 0;
}

/*
 * Define the outlines for a virtual UART IODEV. Other fields will be
 * filled in on instantiation.
 */
static struct r5sim_iodev virtual_disk = {
	.name      = "vdisk",

	/*
	 * 4KB should be more than enough reg space for this device.
	 */
	.io_size   = 0x1000,

	.readl     = virt_disk_readl,
	.writel    = virt_disk_writel,
};

struct r5sim_iodev *r5sim_vdisk_load_new(
	struct r5sim_machine *mach,
	u32 io_offs, const char *path)
{
	struct r5sim_iodev *dev;
	struct virt_disk_priv *priv;

	dev = malloc(sizeof(*dev));
	r5sim_assert(dev != NULL);

	*dev = virtual_disk;

	priv = malloc(sizeof(*priv));
	r5sim_assert(priv != NULL);

	dev->mach = mach;
	dev->io_offset = io_offs;
	dev->priv = priv;

	vdisk_load(mach, priv, path);

	r5sim_info("VDISK @ 0x%x: path=%s\n", io_offs, path);
	r5sim_info("  Disk size: %zu\n", priv->size);

	return dev;
}
