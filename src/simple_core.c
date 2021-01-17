/*
 * Simple RISC-V core implementation. This doesn't do any sort of clever
 * stuff, it just decodes each instruction, executes it, and goes on to
 * the next.
 */

#include <string.h>
#include <stdlib.h>

#include <r5sim/log.h>
#include <r5sim/isa.h>
#include <r5sim/env.h>
#include <r5sim/core.h>
#include <r5sim/util.h>
#include <r5sim/machine.h>
#include <r5sim/simple_core.h>

static void
simple_core_err_dump(struct r5sim_core *core)
{
	r5sim_err("Core error detected!\n");
	r5sim_core_describe(core);
}

static void
simple_core_inst_decode_err(const r5_inst *__inst)
{
	uint32_t *inst = (uint32_t *)__inst;

	r5sim_err("Failed to decode instruction: 0x%08x\n", *inst);
}

static int
exec_misc_mem(struct r5sim_machine *mach,
	      struct r5sim_core *core,
	      const r5_inst *__inst)
{
	r5sim_itrace("NO-OP\n");

	/*
	 * For the simple core these fence operations are just no-ops.
	 */
	return 0;
}

static int
exec_load(struct r5sim_machine *mach,
	  struct r5sim_core *core,
	  const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	uint32_t paddr_src;
	uint32_t w;

	paddr_src = core->reg_file[inst->rs1] +
		sign_extend(inst->imm_11_0, 11);

	/*
	 * Handle the various forms of load.
	 */
	switch (inst->func3) {
	case 0x0: /* LB */
		w = mach->memload8(mach, paddr_src);
		w = sign_extend(w, 7);
		__set_reg(core, inst->rd, w);
		break;
	case 0x1: /* LH */
		w = mach->memload16(mach, paddr_src);
		w = sign_extend(w, 15);
		__set_reg(core, inst->rd, w);
		break;
	case 0x2: /* LW */
		w = mach->memload32(mach, paddr_src);
		__set_reg(core, inst->rd, w);
		break;
	case 0x4: /* LBU */
		w = mach->memload8(mach, paddr_src);
		__set_reg(core, inst->rd, w);
		break;
	case 0x5: /* LHU */
		w = mach->memload16(mach, paddr_src);
		__set_reg(core, inst->rd, w);
		break;
	default:
		simple_core_inst_decode_err(__inst);
		return -1;
	}

	r5sim_itrace("%-6s @ 0x%08x [imm=0x%x] rs=%s\n",
		     r5sim_load_func3_to_str(inst->func3),
		     paddr_src,
		     sign_extend(inst->imm_11_0, 11),
		     r5sim_reg_to_str(inst->rs1));

	return 0;
}

static int
exec_store(struct r5sim_machine *mach,
	    struct r5sim_core *core,
	    const r5_inst *__inst)
{
	const r5_inst_s *inst = (const r5_inst_s *)__inst;
	uint32_t imm;
	uint32_t paddr_dst;

	imm = sign_extend((inst->imm_11_5 << 5) | inst->imm_4_0, 11);
	paddr_dst = __get_reg(core, inst->rs1) + imm;

	switch (inst->func3) {
	case 0x0: /* SB */
		mach->memstore8(mach, paddr_dst,
				core->reg_file[inst->rs2]);
		break;
	case 0x1: /* SH */
		mach->memstore16(mach, paddr_dst,
				 core->reg_file[inst->rs2]);
		break;
	case 0x2: /* SW */
		mach->memstore32(mach, paddr_dst,
				 core->reg_file[inst->rs2]);
		break;
	default:
		simple_core_inst_decode_err(__inst);
		return -1;
	}

	r5sim_itrace("%-6s @ 0x%08x [imm=0x%x] rs=%-3s rd=%-3s\n",
		     r5sim_store_func3_to_str(inst->func3),
		     paddr_dst,
		     imm,
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rs2));

	return 0;
}

static int
exec_op_imm(struct r5sim_machine *mach,
	    struct r5sim_core *core,
	    const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	uint32_t imm = sign_extend(inst->imm_11_0, 11);
	int32_t signed_imm = (int32_t)imm;

	switch (inst->func3) {
	case 0x0: /* ADDI */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) + imm);
		break;
	case 0x1: /* SLLI */
		__set_reg(core, inst->rd,
			  core->reg_file[inst->rs1] << (imm & 0x1f));
		break;
	case 0x2: /* SLTI */
		__set_reg(core, inst->rd,
			  ((int32_t)__get_reg(core, inst->rs1)) < signed_imm ? 1 : 0);
		break;
	case 0x3: /* SLTIU */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) < imm ? 1 : 0);
		break;
	case 0x4: /* XORI */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) ^ imm);
		break;
	case 0x5: /* SRLI, SRAI */
		/*
		 * Logical vs arithmetic shifts; sigh. The logical varient is
		 * easy. The arithmetic a bit more annoying.
		 */
		if (inst->imm_11_0 & (1 << 9))
			__set_reg(core, inst->rd,
				  __get_reg(core, inst->rs1) >> (imm & 0x1f));
		else
			r5sim_assert(!"Sim bug: SRAI not implemented!");
		break;
	case 0x6: /* ORI */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) | imm);
		break;
	case 0x7: /* ANDI */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) & imm);
		break;
	}

	r5sim_itrace("%-6s %-3s [imm=0x%x]\n",
		     r5sim_op_imm_func3_to_str(inst->func3),
		     r5sim_reg_to_str(inst->rs1),
		     imm);
	return 0;
}

static int
exec_op(struct r5sim_machine *mach,
	struct r5sim_core *core,
	const r5_inst *__inst)
{
	const r5_inst_r *inst = (const r5_inst_r *)__inst;

	switch (inst->func3) {
	case 0x0: /* ADD, SUB */
		if (inst->func7 & (0x1 << 5))
			__set_reg(core, inst->rd,
				  __get_reg(core, inst->rs1) -
				  __get_reg(core, inst->rs2));
		else
			__set_reg(core, inst->rd,
				  __get_reg(core, inst->rs1) +
				  __get_reg(core, inst->rs2));
		break;
	case 0x1: /* SLL */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) <<
			  (__get_reg(core, inst->rs2) & 0x1f));
		break;
	case 0x2: /* SLT */
		__set_reg(core, inst->rd,
			  ((int32_t) __get_reg(core, inst->rs1)) <
			  ((int32_t) __get_reg(core, inst->rs2)) ?
			  1 : 0);
		break;
	case 0x3: /* SLTU */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) <
			  __get_reg(core, inst->rs2) ?
			  1 : 0);
		break;
	case 0x4: /* XOR */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) ^
			  __get_reg(core, inst->rs2));
		break;
	case 0x5: /* SRL, SRA */
		if (inst->func7 & (0x1 << 5)) {
			simple_core_inst_decode_err(__inst);
			return -1;
		} else {
			__set_reg(core, inst->rd,
				  __get_reg(core, inst->rs1) >>
				  (__get_reg(core, inst->rs2) & 0x1f));
		}
		break;
	case 0x6: /* OR */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) |
			  __get_reg(core, inst->rs2));
		break;
	case 0x7: /* AND */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) &
			  __get_reg(core, inst->rs2));
		break;
	}

	r5sim_itrace("%-6s %-3s <- %-3s op %-3s\n",
		     r5sim_op_func3_to_str(inst->func3, inst->func7),
		     r5sim_reg_to_str(inst->rd),
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rs2));

	return 0;
}

static int
exec_jal(struct r5sim_machine *mach,
	 struct r5sim_core *core,
	 const r5_inst *__inst)
{
	const r5_inst_j *inst = (const r5_inst_j *)__inst;
	uint32_t lr = core->pc + 4;
	uint32_t offset;

	__set_reg(core, inst->rd, lr);

	offset = (inst->imm_20    << 20) |
		 (inst->imm_19_12 << 12) |
		 (inst->imm_11    << 11) |
		 (inst->imm_10_1  << 1);

	core->pc += sign_extend(offset, 20);

	r5sim_itrace("LR     %-3s [0x%08x] New PC=0x%08x # imm=0x%x\n",
		     r5sim_reg_to_str(inst->rd), lr, core->pc, offset);

	return 0;
}

static int
exec_jalr(struct r5sim_machine *mach,
	  struct r5sim_core *core,
	  const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	uint32_t lr = core->pc + 4;

	/* Set link register. */
	__set_reg(core, inst->rd, lr);

	core->pc = (__get_reg(core, inst->rs1) +
		    sign_extend(inst->imm_11_0, 11)) & ~0x1;

	r5sim_itrace("LR     %-3s [0x%08x] New PC=%08x # rs=%-3s imm=%x\n",
		     r5sim_reg_to_str(inst->rd), lr,
		     core->pc,
		     r5sim_reg_to_str(inst->rs1),
		     sign_extend(inst->imm_11_0, 11) & ~0x1);

	return 0;
}

static int
exec_branch(struct r5sim_machine *mach,
	    struct r5sim_core *core,
	    const r5_inst *__inst)
{
	const r5_inst_b *inst = (const r5_inst_b *)__inst;
	uint32_t rs1, rs2;
	uint32_t offset;
	int take_branch = 0;

	rs1 = __get_reg(core, inst->rs1);
	rs2 = __get_reg(core, inst->rs2);

	switch (inst->func3) {
	case 0x0: /* BEQ */
		take_branch = rs1 == rs2;
		break;
	case 0x1: /* BNE */
		take_branch = rs1 != rs2;
		break;
	case 0x4: /* BLT */
		take_branch = ((int32_t)rs1) < ((int32_t)rs2);
		break;
	case 0x5: /* BGE */
		take_branch = ((int32_t)rs1) >= ((int32_t)rs2);
		break;;
	case 0x6: /* BLTU */
		take_branch = rs1 < rs2;
		break;
	case 0x7: /* BGEU */
		take_branch = rs1 >= rs2;
		break;
	default:
		simple_core_inst_decode_err(__inst);
		return -1;
	}

	/*
	 * If we take the branch increment we are just adding the sign
	 * extended immediate to get to the new PC. However, if we don't
	 * take the branch, remember to increment the PC to the next
	 * instruction!
	 */
	if (take_branch) {
		offset = (inst->imm_12   << 12) |
			 (inst->imm_11   << 11) |
			 (inst->imm_10_5 << 5) |
			 (inst->imm_4_1  << 1);

		core->pc += sign_extend(offset, 12);
	} else {
		core->pc += 4;
	}

	r5sim_itrace("%-6s %-3s [0x%08x] vs %-3s [0x%08x]; New PC=%08x [%-4s] # imm=%x\n",
		     r5sim_branch_func3_to_str(inst->func3),
		     r5sim_reg_to_str(inst->rs1), rs1,
		     r5sim_reg_to_str(inst->rs2), rs2,
		     core->pc,
		     take_branch ? "TAKE" : "SKIP",
		     offset);

	return 0;
}

static int
exec_auipc(struct r5sim_machine *mach,
	   struct r5sim_core *core,
	   const r5_inst *__inst)
{
	const r5_inst_u *inst = (const r5_inst_u *)__inst;
	uint32_t *inst_u32 = (uint32_t *)__inst;

	__set_reg(core, inst->rd,
		  (*inst_u32 & 0xfffff000) + core->pc);

	r5sim_itrace("AIUPC  %-3s <- 0x%08x + 0x%08x",
		     r5sim_reg_to_str(inst->rd),
		     core->pc,
		     (*inst_u32 & 0xfffff000));

	return 0;
}

static int
exec_lui(struct r5sim_machine *mach,
	 struct r5sim_core *core,
	 const r5_inst *__inst)
{
	const r5_inst_u *inst = (const r5_inst_u *)__inst;
	uint32_t *inst_u32 = (uint32_t *)__inst;

	__set_reg(core, inst->rd, *inst_u32 & 0xfffff000);

	r5sim_itrace("LUI    %-3s <- 0x%08x",
		     r5sim_reg_to_str(inst->rd),
		     *inst_u32 & 0xfffff000);

	return 0;
}

static int
exec_system(struct r5sim_machine *mach,
	    struct r5sim_core *core,
	    const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	const uint32_t csr = inst->imm_11_0;

	switch (inst->func3) {
	case 0x1: /* CSRRW */
		__csr_w(core, inst->rd, __get_reg(core, inst->rs1), csr);
		break;
	case 0x2: /* CSRRS */
		__csr_s(core, inst->rd, __get_reg(core, inst->rs1), csr);
		break;
	case 0x3: /* CSRRC */
		__csr_c(core, inst->rd, __get_reg(core, inst->rs1), csr);
		break;
	case 0x5: /* CSRRWI */
		__csr_w(core, inst->rd, inst->rs1, csr);
		break;
	case 0x6: /* CSRRSI */
		__csr_s(core, inst->rd, inst->rs1, csr);
		break;
	case 0x7: /* CSRRCI */
		__csr_c(core, inst->rd, inst->rs1, csr);
		break;
	default:
		simple_core_inst_decode_err(__inst);
		return -1;
	}

	r5sim_itrace("%-6s   0x%3X rd=%-3s %s=%u",
		     r5sim_system_func3_to_str(inst->func3),
		     csr,
		     r5sim_reg_to_str(inst->rd),
		     inst->func3 >= 0x5 ? "imm" : "rs",
		     inst->rs1);

	return 0;
}

/*
 * General instruction table; this defines a function to execute
 * each type of instruction.
 */
static struct r5_op_family op_families[32] = {
	[0]  = R5_OP_FAMILY("LOAD",     R5_OP_TYPE_I, exec_load, 1),
	[1]  = { 0 },
	[2]  = { 0 },
	[3]  = R5_OP_FAMILY("MISC-MEM", R5_OP_TYPE_R, exec_misc_mem, 1),
	[4]  = R5_OP_FAMILY("OP-IMM",   R5_OP_TYPE_I, exec_op_imm, 1),
	[5]  = R5_OP_FAMILY("AUIPC",    R5_OP_TYPE_U, exec_auipc, 1),
	[6]  = { 0 },
	[7]  = { 0 }, /* --- */
	[8]  = R5_OP_FAMILY("STORE",    R5_OP_TYPE_S, exec_store, 1),
	[9]  = { 0 },
	[10] = { 0 },
	[11] = { 0 },
	[12] = R5_OP_FAMILY("OP",       R5_OP_TYPE_R, exec_op, 1),
	[13] = R5_OP_FAMILY("LUI",      R5_OP_TYPE_U, exec_lui, 1),
	[14] = { 0 },
	[15] = { 0 }, /* --- */
	[16] = { 0 },
	[17] = { 0 },
	[18] = { 0 },
	[19] = { 0 },
	[20] = { 0 },
	[21] = { 0 },
	[22] = { 0 },
	[23] = { 0 }, /* --- */
	[24] = R5_OP_FAMILY("BRANCH",   R5_OP_TYPE_B, exec_branch, 0),
	[25] = R5_OP_FAMILY("JALR",     R5_OP_TYPE_I, exec_jalr, 0),
	[26] = { 0 },
	[27] = R5_OP_FAMILY("JAL",      R5_OP_TYPE_J, exec_jal, 0),
	[28] = R5_OP_FAMILY("SYSTEM",   R5_OP_TYPE_I, exec_system, 1),
	[29] = { 0 },
	[30] = { 0 },
	[31] = { 0 }, /* --- */
};

static struct r5_op_family *
simple_core_opcode_fam(r5_inst *inst)
{
	uint32_t type_bits = (inst->opcode & 0x7c) >> 2;

	return &op_families[type_bits];
}

/*
 * Execution happens in the following order:
 *
 *   1. Load the instruction at the current PC;
 *   2. Execute instruction using the op_families decode table.
 *   3. If the instruction is not a BRANCH, JAL, or JALR then
 *      increase PC by 0x4 (i.e move to next instruction). Control flow
 *      already updates the PC, so don't blow that away.
 *
 * The PC increment is done after executing the instruction since the
 * PC may need to be present for certain instructions to execute properly.
 */
static int simple_core_exec_one(struct r5sim_machine *mach,
				struct r5sim_core *core)
{
	uint32_t inst_mem = mach->memload32(mach, core->pc);
	struct r5_op_family *fam;
	uint32_t op_type;
	r5_inst *inst;
	int abort;

	inst = (r5_inst *)(&inst_mem);
	fam = simple_core_opcode_fam(inst);
	op_type = (inst->opcode & 0x7c) >> 2;

	r5sim_itrace("PC 0x%08x i=0x%08x op=%-3d %-8s | ",
		     core->pc, inst_mem,
		     op_type, fam->op_name);

	if (fam->op_name == NULL || fam->op_exec == NULL) {
		simple_core_inst_decode_err(inst);
		simple_core_err_dump(core);
		return -1;
	}

	if ((inst_mem & 0x3) != 0x3) {
		simple_core_inst_decode_err(inst);
		simple_core_err_dump(core);
		return -1;
	}

	abort = fam->op_exec(mach, core, inst);
	if (abort) {
		simple_core_err_dump(core);
		return -1;
	}

	if (fam->incr_pc)
		core->pc += 4;

	return 0;
}

struct r5sim_core *
r5sim_simple_core_instance(struct r5sim_machine *mach)
{
	struct r5sim_core *core;

	core = malloc(sizeof(*core));
	r5sim_assert(core != NULL);

	memset(core, 0, sizeof(*core));

	core->exec_one = simple_core_exec_one;
	core->mach     = mach;
	core->name     = "simple-core-r5";

	r5sim_core_init_common(core);

	return core;
}
