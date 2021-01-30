/*
 * VDISK shared defines for the HW and for the "driver".
 */

#ifndef __R5SIM_HW_VSYS_H__
#define __R5SIM_HW_VSYS_H__

/*
 * Start and size for the IO and DRAM apertures.
 */
#define VSYS_DRAM_START				0x0
#define VSYS_DRAM_SIZE				0x4
#define VSYS_IO_START				0x8
#define VSYS_IO_SIZE				0xc

/*
 * Timers: each sys will support at least 1 timer. To
 * configure a timer, first specify the interval to wait for
 * in the INTERVAL register. Then write to CONFIG with the
 * desired configration and ACTIVATE set.
 *
 * When the timer fires, it will set the interrupt pending
 * bit in the core and will trigger an interrupt.
 */
#define VSYS_TIMER_CONFIG			0x10
#define VSYS_TIMER_CONFIG_PRECISION		1:0
#define VSYS_TIMER_CONFIG_PRECISION_SECS	0
#define VSYS_TIMER_CONFIG_PRECISION_MSECS	1
#define VSYS_TIMER_CONFIG_PRECISION_USECS	2
#define VSYS_TIMER_CONFIG_PRECISION_NSECS	3

#define VSYS_TIMER_CONFIG_ACTIVATE		31:31
#define VSYS_TIMER_CONFIG_ACTIVATE_TRIGGER	1

#define VSYS_TIMER_INTERVAL			0x14
#define VSYS_TIMER_INTERVAL_VALUE		31:0

/*
 * A non-zero write to this register will trigger a SW
 * interrupt in machine mode.
 */
#define VSYS_M_SW_INTERRUPT			0x20

#define VSYS_MAX_REG				0x30

#endif
