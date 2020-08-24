/*
 * ISA related defines, structs, etc.
 */

#ifndef __R5SIM_ISA_H__
#define __R5SIM_ISA_H__

#include <stdint.h>

struct r5sim_machine;
struct r5sim_core;

/*
 * R-type instruction.
 */
typedef struct __r5_inst_r {
	uint32_t opcode:7;

	uint32_t rd:5;
	uint32_t func3:3;
	uint32_t rs1:5;
	uint32_t rs2:5;
	uint32_t func7:7;
} r5_inst_r;

typedef struct __r5_inst_i {
	uint32_t opcode:7;

	uint32_t rd:5;
	uint32_t func3:3;
	uint32_t rs1:5;
	uint32_t imm_11_0:12;
} r5_inst_i;

typedef struct __r5_inst_s {
	uint32_t opcode:7;

	uint32_t imm_4_0:5;
	uint32_t func3:3;
	uint32_t rs1:5;
	uint32_t rs2:5;
	uint32_t imm_11_5:7;
} r5_inst_s;

typedef struct __r5_inst_b {
	uint32_t opcode:7;

	uint32_t imm_11:1;
	uint32_t imm_4_1:4;
	uint32_t func3:3;
	uint32_t rs1:5;
	uint32_t rs2:5;
	uint32_t imm_10_5:6;
	uint32_t imm_12:1;
} r5_inst_b;

typedef struct __r5_inst_u {
	uint32_t opcode:7;

	uint32_t rd:5;
	uint32_t imm_31_12:20;
} r5_inst_u;

typedef struct __r5_inst_j {
	uint32_t opcode:7;

	uint32_t rd:5;
	uint32_t imm_19_12:8;
	uint32_t imm_11:1;
	uint32_t imm_10_1:10;
	uint32_t imm_20:1;
} r5_inst_j;

typedef struct __r5_inst {
	uint32_t opcode:7;

	uint32_t inst_data:25;
} r5_inst;

#define R5_OP_TYPE_UNKNOWN	0x0
#define R5_OP_TYPE_R		0x1
#define R5_OP_TYPE_I		0x2
#define R5_OP_TYPE_S		0x3
#define R5_OP_TYPE_B		0x4
#define R5_OP_TYPE_U		0x5
#define R5_OP_TYPE_J		0x6

struct r5_op_family {
	const char       *op_name;
	const uint32_t    op_type;

	int               incr_pc;

	/*
	 * Execute the specific instruction in the specified machine.
	 */
	int (*op_exec)(struct r5sim_machine *mach,
		       struct r5sim_core *core,
		       const r5_inst *inst);
};

#define R5_OP_FAMILY(name, type, exec, __incr_pc)	\
	{						\
		.op_name   = name,			\
		.op_type   = type,			\
		.op_exec   = exec,			\
		.incr_pc   = __incr_pc,			\
	}

#endif
