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
 * RISC-V core abstraction.
 */

#ifndef __R5SIM_CORE_H__
#define __R5SIM_CORE_H__

#include <time.h>
#include <stdint.h>
#include <pthread.h>

#include <r5sim/isa.h>
#include <r5sim/csr.h>
#include <r5sim/mmu.h>
#include <r5sim/list.h>

/*
 * A max depth of 4 seems like a reasonable place to start; if this needs
 * to be increased that's easy enough.
 */
#define R5SIM_TRAP_DEPTH_MAX		4

struct r5sim_machine;

struct r5sim_core {
	const char           *name;

	/*
	 * Set to non-zero if instruction tracing should be enabled.
	 */
	u32                   itrace;

	u32                   reg_file[32];
	u32                   pc;

	struct r5sim_csr      csr_file[4096];

	struct timespec	      start;

	/*
	 * Priv level for this core.
	 */
	u32                   priv;
#define RV_PRIV_M	      0x3
#define RV_PRIV_S	      0x1
#define RV_PRIV_U	      0x0

	/*
	 * Interrupt related fields for machine mode; currently this is
	 * all we support, so that's all there is.
	 *
	 * To signal an interrupt an external device should call the
	 * r5sim_core_signal_intr() function. This will handle signaling
	 * the relevant core.
	 */
	u32                   mie;
	u32                   mip;
	u32                   medeleg;
	u32                   mideleg;

	u32                   mstatus;

	/*
	 * Memory management unit; includes both the PMP and (future) page
	 * table management.
	 */
	struct r5sim_mmu      mmu;

	/*
	 * Machine this core belongs to; relevant for handling IO, DRAM
	 * access, etc.
	 */
	struct r5sim_machine *mach;

	/*
	 * Ask the core to execute _one_ instruction.
	 *
	 * If this instruction encounters a fault, then the fault should
	 * be returned and _no_ changes to the core's state should occur.
	 * The fault will be handled and then the regular stream of
	 * instructions will continue.
	 */
	int (*exec_one)(struct r5sim_machine *mach,
			struct r5sim_core *core);
};

/*
 * A single, pending interrupt; each core maintais a list of these.
 * Interrupt sources may enqueue an interrupt; the core will dequeue
 * interrupts after each instruction executes.
 */
struct r5sim_core_interrupt {
	u32              cause;

	struct list_head intr_node;
};

static inline void __set_reg(
	struct r5sim_core *core, u32 reg, u32 val)
{
	r5sim_assert(reg < 32);

	/* Silently drop writes to x0 (zero). */
	if (reg == 0)
		return;

	core->reg_file[reg] = val;
}

static inline u32 __get_reg(
	struct r5sim_core *core, u32 reg)
{
	r5sim_assert(reg < 32);

	return core->reg_file[reg];
}

void r5sim_core_init_common(struct r5sim_core *core);
void r5sim_core_exec(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     u32 nr);
void r5sim_core_describe(struct r5sim_core *core);

void r5sim_core_incr(struct r5sim_core *core);
void r5sim_core_wfi(struct r5sim_core *core);
int  r5sim_core_handle_intr(struct r5sim_core *core);
void r5sim_core_intr_signal(struct r5sim_core *core, u32 src);
void __r5sim_core_push_trap(struct r5sim_core *core,
			    u32 priv, u32 code, u32 intr);

const char *r5sim_reg_to_abi_str(u32 reg);
const char *r5sim_reg_to_str(u32 reg);
const char *r5sim_load_func3_to_str(u32 func3);
const char *r5sim_store_func3_to_str(u32 func3);
const char *r5sim_op_imm_func3_to_str(u32 func3);
const char *r5sim_op_i_func3_to_str(u32 func3, u32 func7);
const char *r5sim_op_m_func3_to_str(u32 func3);
const char *r5sim_branch_func3_to_str(u32 func3);
const char *r5sim_system_func3_to_str(u32 func3, u32 csr);

#endif
