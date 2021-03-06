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
 * Machine description and loader.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <r5sim/app.h>
#include <r5sim/env.h>
#include <r5sim/log.h>
#include <r5sim/list.h>
#include <r5sim/core.h>
#include <r5sim/util.h>
#include <r5sim/vdevs.h>
#include <r5sim/iodev.h>
#include <r5sim/machine.h>
#include <r5sim/hwdebug.h>
#include <r5sim/simple_core.h>

/*
 * Given a mask, update only the masked bits in dest with the masked bits
 * in store. This lets us do the byte and half sized loads/stores.
 */
#define store_mask(dest, src, mask)			\
	(dest) = (dest & ~(mask)) | ((src) & (mask))

/*
 * Load a word from a given memory space.
 */
static u32 __load_word(u32 *mem, u32 paddr, u32 base)
{
	return mem[(paddr - base) >> 2];
}

static u32 __load_half(u16 *mem, u32 paddr, u32 base)
{
	return mem[(paddr - base) >> 1];
}

static u32 __load_byte(u8 *mem, u32 paddr, u32 base)
{
	return mem[paddr - base];
}

static void __write_word(u32 *mem, u32 paddr,
			 u32 base, u32 v)
{
	mem[(paddr - base) >> 2] = v;
}

static void __write_half(u16 *mem, u32 paddr,
			 u32 base, u16 v)
{
	mem[(paddr - base) >> 1] = v;
}

static void __write_byte(u8 *mem, u32 paddr,
			 u32 base, u8 v)
{
	mem[paddr - base] = v;
}

/*
 * IO memory is always accessed at 4 byte boundaries.
 */
static int r5sim_default_io_memload(struct r5sim_machine *mach,
				    u32 paddr, u32 *dest)
{
	struct r5sim_iodev *dev;
	u32 io_paddr = paddr - mach->iomem_base;

	/*
	 * Loop through the devices and find a device to access.
	 */
	list_for_each_entry(dev, &mach->io_devs, mach_node) {
		if (!addr_in(dev->io_offset,
			     dev->io_size,
			     io_paddr))
			continue;

		*dest = dev->readl(dev, io_paddr - dev->io_offset);
		return 0;
	}

	/*
	 * No device found!
	 */
	return __ACCESS_FAULT;
}

static int r5sim_default_io_memstore(struct r5sim_machine *mach,
				     u32 paddr,
				     u32 value)
{
	struct r5sim_iodev *dev;
	u32 io_paddr = paddr - mach->iomem_base;

	/*
	 * Loop through the devices and find a device to access.
	 */
	list_for_each_entry(dev, &mach->io_devs, mach_node) {
		if (!addr_in(dev->io_offset,
			     dev->io_size,
			     io_paddr))
			continue;

		dev->writel(dev, io_paddr - dev->io_offset, value);
		return 0;
	}

	/*
	 * No device found!
	 */
	return __ACCESS_FAULT;
}

static int r5sim_default_memload32(struct r5sim_machine *mach,
				   u32 paddr,
				   u32 *dest)
{
	/*
	 * Check alignment; we don't support unaligned loads.
	 */
	if (paddr & 0x3)
		return __ACCESS_MISALIGN;

	/*
	 * DRAM access and BROM access is easy.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		*dest = __load_word((u32 *)mach->memory,
				    paddr, mach->memory_base);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		*dest =  __load_word((u32 *)mach->brom,
				     paddr, mach->brom_base);
	else if (addr_in(mach->iomem_base,
			 mach->iomem_size,
			 paddr))
		return r5sim_default_io_memload(mach, paddr, dest);
	else
		return __ACCESS_FAULT;

	return 0;
}

static int r5sim_default_memload16(struct r5sim_machine *mach,
				   u32 paddr,
				   u16 *dest)
{
	if (paddr & 0x1)
		return __ACCESS_MISALIGN;

	/* Don't allow non-word aligned IO accesses! */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		*dest = __load_half((u16 *)mach->memory,
				    paddr, mach->memory_base);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		*dest = __load_half((u16 *)mach->brom,
				    paddr, mach->brom_base);
	else
		return __ACCESS_FAULT;

	return 0;
}

static int r5sim_default_memload8(struct r5sim_machine *mach,
				  u32 paddr,
				  u8 *dest)
{
	/* Don't allow non-word aligned IO accesses! */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		*dest = __load_byte(mach->memory,
				    paddr, mach->memory_base);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		*dest = __load_byte(mach->brom,
				    paddr, mach->brom_base);
	else
		return __ACCESS_FAULT;

	return 0;
}

static int r5sim_default_memstore32(struct r5sim_machine *mach,
				    u32 paddr,
				    u32 value)
{
	if (paddr & 0x3)
		return __ACCESS_MISALIGN;

	/*
	 * No stores to BROM!
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		__write_word((u32 *)mach->memory, paddr,
			     mach->memory_base, value);
	else if (addr_in(mach->iomem_base,
			 mach->iomem_size,
			 paddr))
		return r5sim_default_io_memstore(mach, paddr, value);
	else
		return __ACCESS_FAULT;

	return 0;
}

static int r5sim_default_memstore16(struct r5sim_machine *mach,
				    u32 paddr,
				    u16 value)
{
	if (paddr & 0x1)
		return __ACCESS_MISALIGN;

	/*
	 * No stores to BROM or to IO mem when not word aligned.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		__write_half((u16 *)mach->memory, paddr,
			     mach->memory_base, value);
	else
		return __ACCESS_FAULT;

	return 0;
}

static int r5sim_default_memstore8(struct r5sim_machine *mach,
				   u32 paddr,
				   u8 value)
{
	/*
	 * No stores to BROM or to IO mem when not word aligned.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		__write_byte(mach->memory, paddr,
			     mach->memory_base, value);
	else
		return __ACCESS_FAULT;

	return 0;
}

/*
 * A default R5 based machine. Some day these should be loadable and
 * configurable.
 */
static struct r5sim_machine default_machine = {
	.descr.name = "default-r5",

	/*
	 * 256 MB of DRAM starting at 1GB.
	 */
	.memory_base = MB(512),
	.memory_size = MB(256),

	/*
	 * BROM region. BROM will be loaded here automatically before
	 * executing the core.
	 */
	.brom_base = KB(64),
	.brom_size = KB(4),

	/*
	 * IOMem aperture of 32MB for now; this is plenty.
	 */
	.iomem_base  = MB(64),
	.iomem_size  = MB(32),

	.memload32   = r5sim_default_memload32,
	.memload16   = r5sim_default_memload16,
	.memload8    = r5sim_default_memload8,
	.memstore32  = r5sim_default_memstore32,
	.memstore16  = r5sim_default_memstore16,
	.memstore8   = r5sim_default_memstore8,

};

static int r5sim_machine_add_device(struct r5sim_machine *mach,
				    struct r5sim_iodev *dev)
{
	list_add_tail(&dev->mach_node, &mach->io_devs);

	/* TODO: Check that device can be added! */
	return 0;
}

struct r5sim_machine *r5sim_machine_load_default(void)
{
	struct r5sim_app_args *args = r5sim_app_get_args();
	struct r5sim_machine *mach = &default_machine;
	struct r5sim_iodev *vuart, *vsys;

	mach->core = r5sim_simple_core_instance(mach);

	mach->memory = malloc(mach->memory_size);
	r5sim_assert(mach->memory != NULL);

	mach->brom = malloc(mach->brom_size);
	r5sim_assert(mach->brom != NULL);

	INIT_LIST_HEAD(&mach->io_devs);

	/*
	 * VUART device at IO + 0x0.
	 */
	vuart = r5sim_vuart_load_new(mach, 0x0);
	r5sim_assert(vuart != NULL);
	r5sim_assert(r5sim_machine_add_device(mach, vuart) == 0);

	/*
	 * VSYS device at IO + 0x100.
	 */
	vsys = r5sim_vsys_load_new(mach, 0x100);
	r5sim_assert(vsys != NULL);
	r5sim_assert(r5sim_machine_add_device(mach, vsys) == 0);

	/*
	 * VDISK device at IO + 0x1000.
	 */
	if (args->disk_file) {
		struct r5sim_iodev *vdisk =
			r5sim_vdisk_load_new(mach, 0x1000, args->disk_file);

		r5sim_assert(r5sim_machine_add_device(mach, vdisk) == 0);
	}

	return mach;
}

void r5sim_machine_load_brom(struct r5sim_machine *mach)
{
	struct r5sim_app_args *args = r5sim_app_get_args();
	int brom_fd;
	char brom_buf[128];
	u32 brom_offs = 0;

	r5sim_assert(args->bootrom != NULL);

	memset(mach->brom, 0x0, mach->brom_size);

	brom_fd = open(args->bootrom, O_RDONLY);
	if (brom_fd < 0) {
		perror(args->bootrom);
		r5sim_assert(!"Failed to open bootrom file!");
	}

	/*
	 * We have a file; now load at most mach->brom_size bytes into brom.
	 */
	while (1) {
		u32 max_bytes = min(mach->brom_size - brom_offs, 128);
		ssize_t bytes = read(brom_fd, brom_buf, max_bytes);

		if (bytes == 0)
			break;

		/*
		 * Otherwise copy the read bytes into the bootrom but make sure
		 * we only copy up to the mac brom size.
		 */
		memcpy(((char *)mach->brom) + brom_offs,
		       brom_buf, bytes);

		brom_offs += bytes;
		if (brom_offs >= mach->brom_size)
			break;
	}

	close(brom_fd);
}

void r5sim_machine_run(struct r5sim_machine *mach)
{
	r5sim_info("Execution begins @ 0x%08x\n", mach->brom_base);

	/*
	 * We assume that the brom has been loaded. The starting PC is address
	 * 0x0 of the bootrom.
	 */
	mach->core->pc = mach->brom_base;

	while (1) {
		r5sim_core_exec(mach, mach->core, 0);
		r5sim_debug_do_session(mach);
	}
}

void r5sim_machine_print(struct r5sim_machine *mach)
{
	struct r5sim_iodev *dev;

	r5sim_info("Machine description: %s\n", mach->descr.name);
	r5sim_info("  DRAM:        0x%08x + 0x%08x)\n",
		   mach->memory_base, mach->memory_size);
	r5sim_info("  IO Aperture: 0x%08x + 0x%08x)\n",
		   mach->iomem_base, mach->iomem_size);

	r5sim_core_describe(mach->core);

	list_for_each_entry(dev, &mach->io_devs, mach_node) {
		r5sim_iodev_describe(dev);
	}
}
