/*
 * Simple virtual disk that we can use for default machines.
 */

#ifndef __R5SIM_VDISK_H__
#define __R5SIM_VDISK_H__

#include <stdint.h>

struct r5sim_iodev;
struct r5sim_machine;

struct r5sim_iodev *
r5sim_vdisk_load_new(struct r5sim_machine *mach,
		     u32 io_offs, const char *path);

#endif
