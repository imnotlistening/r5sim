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
 * ISA related defines, structs, etc.
 */

#ifndef __R5SIM_ISA_H__
#define __R5SIM_ISA_H__

#include <r5sim/env.h>

struct r5sim_machine;
struct r5sim_core;

/*
 * R-type instruction.
 */
typedef struct __r5_inst_r {
	u32 opcode:7;

	u32 rd:5;
	u32 func3:3;
	u32 rs1:5;
	u32 rs2:5;
	u32 func7:7;
} r5_inst_r;

typedef struct __r5_inst_i {
	u32 opcode:7;

	u32 rd:5;
	u32 func3:3;
	u32 rs1:5;
	u32 imm_11_0:12;
} r5_inst_i;

typedef struct __r5_inst_s {
	u32 opcode:7;

	u32 imm_4_0:5;
	u32 func3:3;
	u32 rs1:5;
	u32 rs2:5;
	u32 imm_11_5:7;
} r5_inst_s;

typedef struct __r5_inst_b {
	u32 opcode:7;

	u32 imm_11:1;
	u32 imm_4_1:4;
	u32 func3:3;
	u32 rs1:5;
	u32 rs2:5;
	u32 imm_10_5:6;
	u32 imm_12:1;
} r5_inst_b;

typedef struct __r5_inst_u {
	u32 opcode:7;

	u32 rd:5;
	u32 imm_31_12:20;
} r5_inst_u;

typedef struct __r5_inst_j {
	u32 opcode:7;

	u32 rd:5;
	u32 imm_19_12:8;
	u32 imm_11:1;
	u32 imm_10_1:10;
	u32 imm_20:1;
} r5_inst_j;

typedef struct __r5_inst {
	u32 opcode:7;

	u32 inst_data:25;
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
	const u32    op_type;

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
