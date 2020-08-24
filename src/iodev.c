/*
 * Simple framework for describing IO devices.
 */

#include <r5sim/log.h>
#include <r5sim/iodev.h>

void
r5sim_iodev_describe(struct r5sim_iodev *dev)
{
	r5sim_info("IO Device: %s\n", dev->name);
	r5sim_info("  Aperture info:\n");
	r5sim_info("    Base: 0x%x\n", dev->io_offset);
	r5sim_info("    Size: 0x%x\n", dev->io_size);
}
