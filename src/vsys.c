/*
 * Define a virtual system IO device that contains information about the
 * system that code running on the system may want.
 */

#include <time.h>
#include <signal.h>
#include <stdlib.h>

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/csr.h>
#include <r5sim/core.h>
#include <r5sim/iodev.h>
#include <r5sim/machine.h>

#include <r5sim/hw/vsys.h>

#define vsys_dbg r5sim_dbg_v

#define VSYS_TIMERS	1

struct vsys_priv {
	u32 dev_state[VSYS_MAX_REG >> 2];
};

static const char *vsys_reg_to_str(u32 reg)
{	static const char *str_reg[] = {
		[VSYS_DRAM_START]	= "VSYS_DRAM_START",
		[VSYS_DRAM_SIZE]	= "VSYS_DRAM_SIZE",
		[VSYS_IO_START]		= "VSYS_IO_START",
		[VSYS_IO_SIZE]		= "VSYS_IO_SIZE",
		[VSYS_TIMER_CONFIG]	= "VSYS_TIMER_CONFIG",
		[VSYS_TIMER_INTERVAL]	= "VSYS_TIMER_ACTIVATE",
		[VSYS_M_SW_INTERRUPT]   = "VSYS_M_SW_INTERRUPT",
	};

	r5sim_assert(reg <= VSYS_MAX_REG);

	return str_reg[reg];
}

/*
 * Set a state register for the device.
 */
static void __vsys_set_state(struct vsys_priv *vsys, u32 __i, u32 val)
{
	u32 i = __i >> 2;

	r5sim_assert(i < VSYS_MAX_REG);

	vsys->dev_state[i] = val;
}

static u32 __vsys_read_state(struct vsys_priv *vsys, u32 __i)
{
	u32 i = __i >> 2;

	r5sim_assert(i < VSYS_MAX_REG);

	return vsys->dev_state[i];
}

static void vsys_trigger_msw_intr(struct r5sim_iodev *iodev,
				  u32 value)
{
	struct r5sim_machine *mach = iodev->mach;
	struct r5sim_core *core = mach->core;

	if (value)
		r5sim_core_intr_signal(core, CSR_MCAUSE_CODE_MSI);
}

static u32 vsys_readl(struct r5sim_iodev *iodev, u32 offs)
{
	vsys_dbg("LOAD  @ %s\n", vsys_reg_to_str(offs));

	if (offs >= VSYS_MAX_REG)
		return 0x0;

	return __vsys_read_state(iodev->priv, offs);
}

static void vsys_writel(struct r5sim_iodev *iodev,
		        u32 offs, u32 val)
{
	vsys_dbg("STORE @ %-17s v=0x%08x\n",
		  vsys_reg_to_str(offs), val);

	switch (offs) {
	case VSYS_TIMER_INTERVAL:
		/*
		 * Simple write through.
		 */
		__vsys_set_state(iodev->priv,
				 VSYS_TIMER_INTERVAL,
				 val);
		break;
	case VSYS_TIMER_CONFIG:
		/* Not implemented yet! */
		break;
	case VSYS_M_SW_INTERRUPT:
		vsys_trigger_msw_intr(iodev, val);
		break;
	default:
		vsys_dbg("  Invalid WRITE.");
		return;
	}
}

static struct r5sim_iodev virtual_sys = {
	.name      = "vsys",

	.io_size   = 0x100,

	.readl     = vsys_readl,
	.writel    = vsys_writel,
};

struct r5sim_iodev *r5sim_vsys_load_new(struct r5sim_machine *mach,
					u32 io_offs)
{
	struct r5sim_iodev *dev;
	struct vsys_priv *priv;

	dev = malloc(sizeof(*dev));
	r5sim_assert(dev != NULL);

	priv = malloc(sizeof(*priv));
	r5sim_assert(priv != NULL);

	*dev = virtual_sys;

	dev->mach = mach;
	dev->io_offset = io_offs;
	dev->priv = priv;

	/*
	 * Load our basic settings.
	 */
	__vsys_set_state(priv, VSYS_DRAM_START, mach->memory_base);
	__vsys_set_state(priv, VSYS_DRAM_SIZE,  mach->memory_size);
	__vsys_set_state(priv, VSYS_IO_START,   mach->iomem_base);
	__vsys_set_state(priv, VSYS_IO_SIZE,    mach->iomem_size);

	return dev;
}
