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
 * Manage interrupts pending on a core.
 *
 * The basic idea here is that when an external interrupt source
 * is ready to signal the core, it calls r5sim_core_intr_signal().
 * This tells the core that there's an itnerrutp pending.
 *
 * The core, after each instruction has executed, if interrupts
 * are not masked, checks if there's pending interrupts. If so
 * the oldest interrupt is popped and the core handles the
 * interrupt. A interrupt object is malloc()'ed by the
 * core_signal() function, so the core is expected to free() that
 * interrupt object when it's done with it.
 */

#include <time.h>

#include <r5sim/log.h>
#include <r5sim/core.h>
#include <r5sim/util.h>

/*
 * Wait for an interrupt.
 */
void r5sim_core_wfi(struct r5sim_core *core)
{
	struct timespec spec = {
		.tv_sec  = 0,
		.tv_nsec = 500000, /* .5 milliseconds */
	};

	// volatile typeof(core->mip) *__x = &core->mip;

	/*
	 * Currently we are working with a single core system where
	 * everything but MIP will be constant with respect to a single
	 * instruction. So just wait for MIP to become set. WFI is a hint
	 * and we are free to implement this however we want.
	 *
	 * If there's already a pending interrupt, even if masked or at a
	 * different privilege level, we will break.
	 */
	while (!__VOL_READ(core->mip))
		nanosleep(&spec, NULL);
}

/*
 * Work out if any interrupts are pending and write those interrupt bits
 * into *pending_intr.
 *
 * Will return 0 if no interrupt is pending.
 */
static int __intr_pending(struct r5sim_core *core,
			  u32 *pending_intr, u32 *priv)
{
	u32 smode_intr;
	u32 mmode_intr;

	/*
	 * If no interrupts are pending, then we definitely have nothing
	 * to do. Make sure that the relevant interrupt is actually enabled,
	 * though. And lastly make sure that the interrut is enabled at the
	 * current priv per the mideleg register.
	 */
	if (!(core->mip & core->mie))
		return 0;

	smode_intr = core->mip & core->mie &  core->mideleg;
	mmode_intr = core->mip & core->mie & ~core->mideleg;

	/*
	 * For U/S-mode, we check if a delegated interrupt is pending. But we
	 * _also_ have to check for M-mode interrupts pending as well as these
	 * are always globally enabled when U/S-mode is active.
	 */
	if (core->priv != RV_PRIV_M) {
		if (mmode_intr) {
			*pending_intr = mmode_intr;
			*priv = RV_PRIV_M;
			r5sim_dbg("M-mode intr from S-mode\n");
			return 1;
		}

		if (smode_intr &&
		    get_field(core->mstatus, CSR_MSTATUS_SIE)) {
			*pending_intr = smode_intr;
			*priv = RV_PRIV_S;
			return 1;
		}
	}

	/*
	 * If we are in M-mode so long as M-mode interrupts are globally
	 * enabled and there's a non-delegated interrupt pending we should
	 * handle it.
	 */
	if (mmode_intr &&
	    get_field(core->mstatus, CSR_MSTATUS_MIE)) {
		*pending_intr = mmode_intr;
		*priv = RV_PRIV_M;
		return 1;
	}

	return 0;
}

/*
 * Handle a potential interrupt; if there was an interrupt this will take
 * care of everything necessary such that the next instruction executed
 * is the interrupt trap vector.
 *
 * If it returns true, there is an interrupt; false otherwise.
 */
int r5sim_core_handle_intr(struct r5sim_core *core)
{
	u32 intr, priv;
	u32 code;

	if (!__intr_pending(core, &intr, &priv))
		return 0;

	r5sim_core_incr(core);

	/*
	 * We have an interrupt, figure out which one to take. For now
	 * just take the lowest bit first.
	 */
	r5sim_assert(intr != 0);
	code = __builtin_ffs(intr) - 1;

	__r5sim_core_push_trap(core, priv, code, 1);

	return 1;
}

void r5sim_core_intr_signal(struct r5sim_core *core, u32 src)
{
	r5sim_dbg("Interrupt reported: %u\n", src);

	core->mip |= (1 << src);
}
