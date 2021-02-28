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
#include <r5sim/util.h>
#include <r5sim/iodev.h>
#include <r5sim/machine.h>

#include <r5sim/hw/vsys.h>

#define vsys_dbg r5sim_dbg_v

#define VSYS_TIMERS	1

struct vsys_priv {
	u32             dev_state[VSYS_MAX_REG >> 2];

	timer_t         timer;
	struct sigevent ev;
};

static const char *vsys_reg_to_str(u32 reg)
{	static const char *str_reg[] = {
		[VSYS_DRAM_START]	= "VSYS_DRAM_START",
		[VSYS_DRAM_SIZE]	= "VSYS_DRAM_SIZE",
		[VSYS_IO_START]		= "VSYS_IO_START",
		[VSYS_IO_SIZE]		= "VSYS_IO_SIZE",
		[VSYS_TIMER_CONFIG]	= "VSYS_TIMER_CONFIG",
		[VSYS_TIMER_INTERVAL]	= "VSYS_TIMER_INTERVAL",
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

static void vsys_timer_notify(union sigval sv)
{
	struct r5sim_core *core = sv.sival_ptr;

	r5sim_dbg("Timer fired!\n");

	r5sim_core_intr_signal(core, CSR_MCAUSE_CODE_MTI);
}

static void vsys_trigger_timer(struct vsys_priv *vsys)
{
	struct itimerspec spec = { };
	u64 interv = __vsys_read_state(vsys, VSYS_TIMER_INTERVAL);
	u32 config = __vsys_read_state(vsys, VSYS_TIMER_CONFIG);
	u32 prec = get_field(config, VSYS_TIMER_CONFIG_PRECISION);

	/*
	 * Scale the incoming time to nanoseconds; then we'll fill
	 * in the timer spec with the right second/ns split.
	 */
	switch (prec) {
	case VSYS_TIMER_CONFIG_PRECISION_SECS:
		interv *= 1000000000;
		break;
	case VSYS_TIMER_CONFIG_PRECISION_MSECS:
		interv *= 1000000;
		break;
	case VSYS_TIMER_CONFIG_PRECISION_USECS:
		interv *= 1000;
		break;
	}

	spec.it_value.tv_sec  = interv ? interv / 1000000000 : 0u;
	spec.it_value.tv_nsec = interv % 1000000000;

	timer_settime(vsys->timer, 0, &spec, NULL);
	r5sim_dbg("Timer activated: [%ld.%ld]\n",
		  spec.it_value.tv_sec,
		  spec.it_value.tv_nsec);
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
		__vsys_set_state(iodev->priv,
				 VSYS_TIMER_CONFIG,
				 val);

		/* Now (potentially) trigger the timer. */
		if (get_field(val, VSYS_TIMER_CONFIG_ACTIVATE))
			vsys_trigger_timer(iodev->priv);
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

	/*
	 * Init timer for the vsys.
	 */
	priv->ev.sigev_notify = SIGEV_THREAD;
	priv->ev.sigev_value.sival_ptr = mach->core;
	priv->ev.sigev_notify_function = vsys_timer_notify;

	if (timer_create(CLOCK_REALTIME, &priv->ev, &priv->timer))
		r5sim_assert(!"timer_create: failed!");

	return dev;
}
