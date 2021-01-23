/*
 * RISC-V core abstraction.
 */

#ifndef __R5SIM_CORE_H__
#define __R5SIM_CORE_H__

#include <time.h>
#include <stdint.h>

#include <r5sim/isa.h>
#include <r5sim/csr.h>

struct r5sim_machine;
struct r5sim_core;

struct r5sim_core {
	const char           *name;

	u32                   reg_file[32];
	u32                   pc;

	struct r5sim_csr      csr_file[4096];

	struct timespec	      start;

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
		     u32 pc);
void r5sim_core_describe(struct r5sim_core *core);

/*
 * Ask the core to execute a trap.
 *
 * This will execute a series of .exec_one() calls, corresponding
 * to the trap handler. When this returns the main execution loop
 * will continue executing, but with a potentially new PC.
 */
int r5sim_core_trap(struct r5sim_machine *mach,
		    struct r5sim_core *core,
		    u32 code);

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
