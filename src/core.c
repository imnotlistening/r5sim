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

const char *
r5sim_reg_to_abi_str(uint32_t reg)
{
	return reg_abi_names[reg];
}

const char *
r5sim_reg_to_str(uint32_t reg)
{
	return reg_names[reg];
}

const char *
r5sim_load_func3_to_str(uint32_t func3)
{
	switch (func3) {
	case 0x0: /* LB */
		return "LB";
	case 0x1: /* LH */
		return "LH";
	case 0x2: /* LW */
		return "LW";
	case 0x4: /* LBU */
		return "LBU";
	case 0x5: /* LHU */
		return "LHU";
	}

	return "ERR";
};

const char *
r5sim_store_func3_to_str(uint32_t func3)
{
	switch (func3) {
	case 0x0: /* SB */
		return "LB";
	case 0x1: /* SH */
		return "LH";
	case 0x2: /* SW */
		return "LW";
	}

	return "ERR";
}

const char *
r5sim_op_imm_func3_to_str(uint32_t func3)
{
	switch (func3) {
	case 0x0: /* ADDI */
		return "ADDI";
	case 0x1: /* SLLI */
		return "SLLI";
	case 0x2: /* SLTI */
		return "SLTI";
	case 0x3: /* SLTIU */
		return "SLTIUI";
	case 0x4: /* XORI */
		return "XORI";
	case 0x5: /* SRLI, SRAI */
		return "SRLI";
	case 0x6: /* ORI */
		return "ORI";
	case 0x7: /* ANDI */
		return "ANDI";
	}

	return "ERR";
}

const char *
r5sim_op_func3_to_str(uint32_t func3, uint32_t func7)
{
	switch (func3) {
	case 0x0: /* ADD, SUB */
		return (func7 & (0x1 << 5)) ? "SUB" : "ADD";
	case 0x1: /* SLL */
		return "SLL";
	case 0x2: /* SLT */
		return "SLT";
	case 0x3: /* SLTU */
		return "SLTU";
	case 0x4: /* XOR */
		return "XOR";
	case 0x5: /* SRL, SRA */
		return "SRA?";
	case 0x6: /* OR */
		return "OR";
	case 0x7: /* AND */
		return "AND";
	}

	return "ERR";

}

const char *
r5sim_branch_func3_to_str(uint32_t func3)
{
	switch (func3) {
	case 0x0: /* BEQ */
		return "BEQ";
	case 0x1: /* BNE */
		return "BNE";
	case 0x4: /* BLT */
		return "BLT";
	case 0x5: /* BGE */
		return "BGR";
	case 0x6: /* BLTU */
		return "BLTU";
	case 0x7: /* BGEU */
		return "BGEU";
	}

	return "ERR";
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
