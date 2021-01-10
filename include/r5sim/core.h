/*
 * RISC-V core abstraction.
 */

#ifndef __R5SIM_CORE_H__
#define __R5SIM_CORE_H__

#include <stdint.h>

#include <r5sim/isa.h>

struct r5sim_machine;

struct r5sim_core {
	const char           *name;

	uint32_t              reg_file[32];
	uint32_t              pc;

	/*
	 * Machine this core belongs to; relevant for handling IO, DRAM
	 * access, etc.
	 */
	struct r5sim_machine *mach;

	/*
	 * Begin execution of this core at the specified start_addr.
	 */
	void (*exec)(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     uint32_t start_addr);
};

static inline void
__set_reg(struct r5sim_core *core,
	  uint32_t reg, uint32_t val)
{
	r5sim_assert(reg < 32);

	/* Silently drop writes to x0 (zero). */
	if (reg == 0)
		return;

	core->reg_file[reg] = val;
}

static inline uint32_t
__get_reg(struct r5sim_core *core, uint32_t reg)
{
	r5sim_assert(reg < 32);

	return core->reg_file[reg];
}

const char *
r5sim_reg_to_abi_str(uint32_t reg);
const char *
r5sim_reg_to_str(uint32_t reg);
const char *
r5sim_load_func3_to_str(uint32_t func3);
const char *
r5sim_store_func3_to_str(uint32_t func3);
const char *
r5sim_op_imm_func3_to_str(uint32_t func3);
const char *
r5sim_op_func3_to_str(uint32_t func3, uint32_t func7);
const char *
r5sim_branch_func3_to_str(uint32_t func3);

void
r5sim_core_describe(struct r5sim_core *core);

#endif
