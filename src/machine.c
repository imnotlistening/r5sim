/*
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
#include <r5sim/vuart.h>
#include <r5sim/vdisk.h>
#include <r5sim/iodev.h>
#include <r5sim/machine.h>
#include <r5sim/simple_core.h>

/*
 * Given a mask, update only the masked bits in dest with the masked bits
 * in store. This lets us do the byte and half sized loads/stores.
 */
#define store_mask(dest, src, mask)			\
	(dest) = (dest & ~(mask)) | ((src) & (mask))

/*
 * Handle a load from an arbitrary (potentially unaligned) address
 * in the specified mem region. paddr needs to be normalized to the
 * mem address space. bytes is maxed at 4 (e.g sizeof(uint32_t)).
 */
static uint32_t
__load_bytes(uint8_t *mem, uint32_t paddr, uint32_t bytes)
{
	uint32_t w;

	r5sim_assert(bytes && bytes <= sizeof(w));

	memcpy(&w, &mem[paddr], bytes);

	return w;
}

/*
 * Write number of bytes to an arbitrary address. The max bytes is 4
 * and are copied from val.
 */
static void
__write_bytes(uint8_t *mem, uint32_t paddr,
			      uint32_t val, uint32_t bytes)
{
	r5sim_assert(bytes && bytes <= sizeof(val));

	memcpy(&mem[paddr], &val, bytes);
}

/*
 * IO memory is always accessed at 4 byte boundaries.
 */
static uint32_t
r5sim_default_io_memload(struct r5sim_machine *mach,
			 uint32_t paddr)
{
	struct r5sim_iodev *dev;
	uint32_t io_paddr = paddr - mach->iomem_base;

	/*
	 * Loop through the devices and find a device to access.
	 */
	list_for_each_entry(dev, &mach->io_devs, mach_node) {
		if (addr_in(dev->io_offset,
			    dev->io_size,
			    io_paddr))
			return dev->readl(dev, io_paddr - dev->io_offset);
	}

	/*
	 * No device found!
	 */
	r5sim_warn("Load to non-existent IO addr: 0x%08x\n", paddr);

	return 0x0;
}

static void
r5sim_default_io_memstore(struct r5sim_machine *mach,
			  uint32_t paddr,
			  uint32_t value)
{
	struct r5sim_iodev *dev;
	uint32_t io_paddr = paddr - mach->iomem_base;

	/*
	 * Loop through the devices and find a device to access.
	 */
	list_for_each_entry(dev, &mach->io_devs, mach_node) {
		if (addr_in(dev->io_offset,
			    dev->io_size,
			    io_paddr)) {
			dev->writel(dev, io_paddr - dev->io_offset, value);
			return;
		}
	}

	/*
	 * No device found!
	 */
	r5sim_warn("Store to non-existent IO addr: 0x%08x\n", paddr);
}

static uint32_t
r5sim_default_memload32(struct r5sim_machine *mach,
			uint32_t paddr)
{
	/* Align to 4bytes. */
	paddr &= ~0x3;

	/*
	 * DRAM access and BROM access is easy.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		return __load_bytes(mach->memory, paddr - mach->memory_base, 0x4);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		return __load_bytes(mach->brom, paddr - mach->brom_base, 0x4);
	else if (addr_in(mach->iomem_base,
			 mach->iomem_size,
			 paddr))
		return r5sim_default_io_memload(mach, paddr);
	else {
		r5sim_core_describe(mach->core);
		r5sim_err("Invalid load address: 0x%x\n", paddr);
		r5sim_assert(!"Invalid LOAD address!");
	}

	return 0x0;
}

static uint16_t
r5sim_default_memload16(struct r5sim_machine *mach,
			uint32_t paddr)
{
	/* Align to 2bytes. */
	paddr &= ~0x1;

	/* Don't allow non-word aligned IO accesses! */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		return __load_bytes(mach->memory, paddr - mach->memory_base, 0x2);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		return __load_bytes(mach->brom, paddr - mach->brom_base, 0x2);
	else {
		r5sim_core_describe(mach->core);
		r5sim_err("16b access to IO aperture: 0x%x\n", paddr);
		r5sim_assert(!"Invalid LOAD address!");
	}

	return 0x0;
}

static uint8_t
r5sim_default_memload8(struct r5sim_machine *mach,
		       uint32_t paddr)
{
	/* Don't allow non-word aligned IO accesses! */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		return __load_bytes(mach->memory, paddr - mach->memory_base, 0x1);
	else if (addr_in(mach->brom_base,
			 mach->brom_size,
			 paddr))
		return __load_bytes(mach->brom, paddr - mach->brom_base, 0x1);
	else {
		r5sim_core_describe(mach->core);
		r5sim_err("8b access to IO aperture: 0x%x\n", paddr);
		r5sim_assert(!"Invalid LOAD address!");
	}

	return 0;
}

static void
r5sim_default_memstore32(struct r5sim_machine *mach,
			 uint32_t paddr,
			 uint32_t value)
{
	/* Align to 4 bytes. */
	paddr &= ~0x3;

	/*
	 * No stores to BROM!
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr))
		__write_bytes(mach->memory, paddr - mach->memory_base,
			      value, 0x4);
	else if (addr_in(mach->iomem_base,
			 mach->iomem_size,
			 paddr))
		r5sim_default_io_memstore(mach, paddr, value);
	else {
		r5sim_core_describe(mach->core);
		r5sim_err("Invalid store address: 0x%x\n", paddr);
		r5sim_assert(!"Invalid STORE address!");
	}
}

static void
r5sim_default_memstore16(struct r5sim_machine *mach,
			 uint32_t paddr,
			 uint16_t value)
{
	/* Align to 2 bytes. */
	paddr &= ~0x1;

	/*
	 * No stores to BROM or to IO mem when not word aligned.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr)) {
		__write_bytes(mach->memory, paddr - mach->memory_base,
			      value, 0x2);
	} else {
		r5sim_core_describe(mach->core);
		r5sim_err("Invalid store address: 0x%x\n", paddr);
		r5sim_assert(!"Invalid STORE address!");
	}
}

static void
r5sim_default_memstore8(struct r5sim_machine *mach,
			 uint32_t paddr,
			 uint8_t value)
{
	/*
	 * No stores to BROM or to IO mem when not word aligned.
	 */
	if (addr_in(mach->memory_base,
		    mach->memory_size,
		    paddr)) {
		__write_bytes(mach->memory, paddr - mach->memory_base,
			      value, 0x1);
	} else {
		r5sim_core_describe(mach->core);
		r5sim_err("Invalid store address: 0x%x\n", paddr);
		r5sim_assert(!"Invalid STORE address!");
	}
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
	.memory_base = GB(1),
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

static int
r5sim_machine_add_device(struct r5sim_machine *mach,
			 struct r5sim_iodev *dev)
{
	list_add_tail(&dev->mach_node, &mach->io_devs);

	/* TODO: Check that device can be added! */
	return 0;
}

struct r5sim_machine *
r5sim_machine_load_default(void)
{
	struct r5sim_app_args *args = r5sim_app_get_args();
	struct r5sim_machine *mach = &default_machine;
	struct r5sim_iodev *vuart;

	mach->core = r5sim_simple_core_instance(mach);

	mach->memory = malloc(mach->memory_size);
	mach->memory_words = (uint32_t *)mach->memory;
	r5sim_assert(mach->memory != NULL);

	mach->brom = malloc(mach->brom_size);
	mach->brom_words = (uint32_t *)mach->brom;
	r5sim_assert(mach->brom != NULL);

	INIT_LIST_HEAD(&mach->io_devs);

	vuart = r5sim_vuart_load_new(mach, 0x0);
	r5sim_assert(vuart != NULL);

	r5sim_assert(r5sim_machine_add_device(mach, vuart) == 0);

	if (args->disk_file) {
		struct r5sim_iodev *vdisk =
			r5sim_vdisk_load_new(mach, 0x1000, args->disk_file);

		r5sim_assert(r5sim_machine_add_device(mach, vdisk) == 0);
	}

	return mach;
}

void
r5sim_machine_load_brom(struct r5sim_machine *mach)
{
	struct r5sim_app_args *args = r5sim_app_get_args();
	int brom_fd;
	char brom_buf[128];
	uint32_t brom_offs = 0;

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
		uint32_t max_bytes = min(mach->brom_size - brom_offs, 128);
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

void
r5sim_machine_boot(struct r5sim_machine *mach)
{
	/*
	 * We assume that the brom has been loaded. The starting PC is address
	 * 0x0 of the bootrom.
	 */
	r5sim_core_exec(mach, mach->core, mach->brom_base);
}

void
r5sim_machine_print(struct r5sim_machine *mach)
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
