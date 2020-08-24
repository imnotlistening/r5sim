/*
 * Basic CPU core interfaces.
 */

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/core.h>

static const char *reg_abi_names[32] = {
	" z", /* 0 */
	"ra",
	"sp",
	"gp",
	"tp",
	"t0",
	"t1",
	"t2",
	"fp", /* 8 */
	"s1",
	"a0",
	"a1",
	"a2",
	"a3",
	"a4",
	"a5", /* 15 */
	"a6",
	"a7",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7", /* 23 */
	"s8",
	"s9",
	"s10",
	"s11",
	"t3",
	"t4",
	"t5",
	"t6",
};

static const char *reg_names[32] = {
	"x0",
	"x1",
	"x2",
	"x3",
	"x4",
	"x5",
	"x6",
	"x7",
	"x8",
	"x9",
	"x10",
	"x11",
	"x12",
	"x13",
	"x14",
	"x15",
	"x16",
	"x17",
	"x18",
	"x19",
	"x20",
	"x21",
	"x22",
	"x23",
	"x24",
	"x25",
	"x26",
	"x27",
	"x28",
	"x29",
	"x30",
	"x31",
};

const char *r5sim_reg_to_abi_str(uint32_t reg)
{
	return reg_abi_names[reg];
}

const char *r5sim_reg_to_str(uint32_t reg)
{
	return reg_names[reg];
}

void
r5sim_core_describe(struct r5sim_core *core)
{
	int i;

	r5sim_info("Core: %s\n", core->name);
	r5sim_info(" PC: 0x%08x\n", core->pc);

	for (i = 0; i < 32; i += 4) {
		r5sim_info("%3s: 0x%08x  %3s: 0x%08x  %3s: 0x%08x  %3s: 0x%08x\n",
			   r5sim_reg_to_str(i + 0), core->reg_file[i + 0],
			   r5sim_reg_to_str(i + 1), core->reg_file[i + 1],
			   r5sim_reg_to_str(i + 2), core->reg_file[i + 2],
			   r5sim_reg_to_str(i + 3), core->reg_file[i + 3]);
	}
}
