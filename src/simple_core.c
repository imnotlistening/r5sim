/*
 * Simple RISC-V core implementation. This doesn't do any sort of clever
 * stuff, it just decodes each instruction, executes it, and goes on to
 * the next.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <r5sim/app.h>
#include <r5sim/log.h>
#include <r5sim/isa.h>
#include <r5sim/env.h>
#include <r5sim/core.h>
#include <r5sim/trap.h>
#include <r5sim/util.h>
#include <r5sim/machine.h>
#include <r5sim/simple_core.h>

static int exec_misc_mem(struct r5sim_machine *mach,
			 struct r5sim_core *core,
			 const r5_inst *__inst)
{
	r5sim_itrace("NO-OP\n");

	/*
	 * For the simple core these fence operations are just no-ops.
	 */
	return 0;
}

static int exec_load(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	u32 paddr_src;
	u32 w = 0;

	paddr_src = core->reg_file[inst->rs1] +
		sign_extend(inst->imm_11_0, 11);

	/*
	 * Handle the various forms of load.
	 */
	switch (inst->func3) {
	case 0x0: /* LB */
		/* Since there's no MMU this can't really trap... */
		if (mach->memload8(mach, paddr_src, (u8 *)(&w)))
			return TRAP_LD_ADDR_MISLAIGN;
		w = sign_extend(w, 7);
		__set_reg(core, inst->rd, w);
		break;
	case 0x1: /* LH */
		if (mach->memload16(mach, paddr_src, (u16 *)(&w)))
			return TRAP_LD_ADDR_MISLAIGN;
		w = sign_extend(w, 15);
		__set_reg(core, inst->rd, w);
		break;
	case 0x2: /* LW */
		if (mach->memload32(mach, paddr_src, &w))
			return TRAP_LD_ADDR_MISLAIGN;
		__set_reg(core, inst->rd, w);
		break;
	case 0x4: /* LBU */
		if (mach->memload8(mach, paddr_src, (u8 *)(&w)))
			return TRAP_LD_ADDR_MISLAIGN;
		__set_reg(core, inst->rd, w);
		break;
	case 0x5: /* LHU */
		if (mach->memload16(mach, paddr_src, (u16 *)(&w)))
			return TRAP_LD_ADDR_MISLAIGN;
		__set_reg(core, inst->rd, w);
		break;
	default:
		return TRAP_ILLEGAL_INST;
	}

	r5sim_itrace("%-6s @ 0x%08x [imm=0x%x] rs=%-3s rd=%s\n",
		     r5sim_load_func3_to_str(inst->func3),
		     paddr_src,
		     sign_extend(inst->imm_11_0, 11),
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rd));

	return 0;
}

static int exec_store(struct r5sim_machine *mach,
		      struct r5sim_core *core,
		      const r5_inst *__inst)
{
	const r5_inst_s *inst = (const r5_inst_s *)__inst;
	u32 imm;
	u32 paddr_dst;

	imm = sign_extend((inst->imm_11_5 << 5) | inst->imm_4_0, 11);
	paddr_dst = __get_reg(core, inst->rs1) + imm;

	switch (inst->func3) {
	case 0x0: /* SB */
		if (mach->memstore8(mach, paddr_dst,
				    (u8)core->reg_file[inst->rs2]))
			return TRAP_LD_ADDR_MISLAIGN;
		break;
	case 0x1: /* SH */
		if (mach->memstore16(mach, paddr_dst,
				     (u16)core->reg_file[inst->rs2]))
		    return TRAP_LD_ADDR_MISLAIGN;
		break;
	case 0x2: /* SW */
		if (mach->memstore32(mach, paddr_dst,
				     core->reg_file[inst->rs2]))
			return TRAP_LD_ADDR_MISLAIGN;
		break;
	default:
		return TRAP_ILLEGAL_INST;
	}

	r5sim_itrace("%-6s @ 0x%08x [imm=0x%x] rs=%-3s rd=%-3s\n",
		     r5sim_store_func3_to_str(inst->func3),
		     paddr_dst,
		     imm,
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rs2));

	return 0;
}

static int exec_op_imm(struct r5sim_machine *mach,
		       struct r5sim_core *core,
		       const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	u32 imm = sign_extend(inst->imm_11_0, 11);
	s32 signed_imm = (s32)imm;

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
			  ((s32)__get_reg(core, inst->rs1)) < signed_imm ? 1 : 0);
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
		 * Most C compilers, apparently, implement logical right shifts
		 * for signed types.
		 * A quick local test verified this to be true for me - but YMMV.
		 */
		if (inst->imm_11_0 & (1 << 9))
			__set_reg(core, inst->rd,
				  ((s32)__get_reg(core, inst->rs1)) >>
				  (imm & 0x1f));
		else
			__set_reg(core, inst->rd,
				  __get_reg(core, inst->rs1) >> (imm & 0x1f));
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

	r5sim_itrace("%-6s %-3s <- %-3s [imm=0x%x]\n",
		     r5sim_op_imm_func3_to_str(inst->func3),
		     r5sim_reg_to_str(inst->rd),
		     r5sim_reg_to_str(inst->rs1),
		     imm);
	return 0;
}

static int exec_op_i(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     const r5_inst_r *inst)
{
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
			  ((s32) __get_reg(core, inst->rs1)) <
			  ((s32) __get_reg(core, inst->rs2)) ?
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
		if (inst->func7 & (0x1 << 5)) { /* SRA */
			/*
			 * Most C compilers, apparently, do arithmetic
			 * shifting on signed types.
			 */
			__set_reg(core, inst->rd,
				  ((s32)__get_reg(core, inst->rs1)) >>
				  (__get_reg(core, inst->rs2) & 0x1f));
		} else { /* SRL */
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
		     r5sim_op_i_func3_to_str(inst->func3, inst->func7),
		     r5sim_reg_to_str(inst->rd),
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rs2));

	return 0;
}

static int exec_op_m(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     const r5_inst_r *inst)
{
	uint64_t uproduct;
	int64_t  sproduct;

	switch (inst->func3) {
	case 0x0: /* MUL */
		sproduct = (s32)__get_reg(core, inst->rs1) *
			   (s32)__get_reg(core, inst->rs2);
		__set_reg(core, inst->rd, (u32)(sproduct & 0xffffffff));
		break;
	case 0x1: /* MULH */
		sproduct = sign_extend_64(__get_reg(core, inst->rs1), 31) *
			   sign_extend_64(__get_reg(core, inst->rs2), 31);
		__set_reg(core, inst->rd,
			  (u32)((sproduct >> 32) & 0xffffffff));
		break;
	case 0x2: /* MULHSU */
		sproduct = sign_extend_64(__get_reg(core, inst->rs1), 31) *
			   __get_reg(core, inst->rs2);
		__set_reg(core, inst->rd,
			  (u32)((sproduct >> 32) & 0xffffffff));
		break;
	case 0x3: /* MULHU */
		uproduct = ((uint64_t)__get_reg(core, inst->rs1)) *
			   ((uint64_t)__get_reg(core, inst->rs2));
		__set_reg(core, inst->rd,
			  (u32)((uproduct >> 32) & 0xffffffff));
		break;
	case 0x4: /* DIV */
		__set_reg(core, inst->rd, (u32)
			  (((s32)__get_reg(core, inst->rs1)) /
			   ((s32)__get_reg(core, inst->rs2))));
		break;
	case 0x5: /* DIVU */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) /
			  __get_reg(core, inst->rs2));
		break;
	case 0x6: /* REM */
		__set_reg(core, inst->rd, (u32)
			  ((s32)__get_reg(core, inst->rs1) %
			   (s32)__get_reg(core, inst->rs2)));
		break;
	case 0x7: /* REMU */
		__set_reg(core, inst->rd,
			  __get_reg(core, inst->rs1) %
			  __get_reg(core, inst->rs2));
		break;
	}

	r5sim_itrace("%-6s %-3s <- %-3s op %-3s\n",
		     r5sim_op_m_func3_to_str(inst->func3),
		     r5sim_reg_to_str(inst->rd),
		     r5sim_reg_to_str(inst->rs1),
		     r5sim_reg_to_str(inst->rs2));

	return 0;
}

static int exec_op(struct r5sim_machine *mach,
		   struct r5sim_core *core,
		   const r5_inst *__inst)
{
	const r5_inst_r *inst = (const r5_inst_r *)__inst;

	if (inst->func7 == 1)
		return exec_op_m(mach, core, inst);
	else
		return exec_op_i(mach, core, inst);
}

static int exec_jal(struct r5sim_machine *mach,
		    struct r5sim_core *core,
		    const r5_inst *__inst)
{
	const r5_inst_j *inst = (const r5_inst_j *)__inst;
	u32 lr = core->pc + 4;
	u32 offset;

	offset = (inst->imm_20    << 20) |
		 (inst->imm_19_12 << 12) |
		 (inst->imm_11    << 11) |
		 (inst->imm_10_1  << 1);

	if (offset & 0x3)
		return TRAP_INST_ADDR_MISALIGN;

	__set_reg(core, inst->rd, lr);

	core->pc += sign_extend(offset, 20);

	r5sim_itrace("LR     %-3s [0x%08x] New PC=0x%08x # imm=0x%x\n",
		     r5sim_reg_to_str(inst->rd), lr, core->pc, offset);

	return 0;
}

static int exec_jalr(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	u32 lr = core->pc + 4;
	u32 target = (__get_reg(core, inst->rs1) +
		      sign_extend(inst->imm_11_0, 11)) & ~0x1;

	if (target & 0x3)
		return TRAP_INST_ADDR_MISALIGN;

	/* Set link register. */
	__set_reg(core, inst->rd, lr);

	core->pc = target;

	r5sim_itrace("LR     %-3s [0x%08x] New PC=%08x # rs=%-3s imm=%x\n",
		     r5sim_reg_to_str(inst->rd), lr,
		     core->pc,
		     r5sim_reg_to_str(inst->rs1),
		     sign_extend(inst->imm_11_0, 11) & ~0x1);

	return 0;
}

static int exec_branch(struct r5sim_machine *mach,
		       struct r5sim_core *core,
		       const r5_inst *__inst)
{
	const r5_inst_b *inst = (const r5_inst_b *)__inst;
	u32 rs1, rs2;
	u32 offset = 0;
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
		take_branch = ((s32)rs1) < ((s32)rs2);
		break;
	case 0x5: /* BGE */
		take_branch = ((s32)rs1) >= ((s32)rs2);
		break;;
	case 0x6: /* BLTU */
		take_branch = rs1 < rs2;
		break;
	case 0x7: /* BGEU */
		take_branch = rs1 >= rs2;
		break;
	default:
		return TRAP_ILLEGAL_INST;
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

		if (offset & 0x3)
			return TRAP_INST_ADDR_MISALIGN;

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

static int exec_auipc(struct r5sim_machine *mach,
		      struct r5sim_core *core,
		      const r5_inst *__inst)
{
	const r5_inst_u *inst = (const r5_inst_u *)__inst;
	u32 *inst_u32 = (u32 *)__inst;

	__set_reg(core, inst->rd,
		  (*inst_u32 & 0xfffff000) + core->pc);

	r5sim_itrace("AIUPC  %-3s <- 0x%08x + 0x%08x\n",
		     r5sim_reg_to_str(inst->rd),
		     core->pc,
		     (*inst_u32 & 0xfffff000));

	return 0;
}

static int exec_lui(struct r5sim_machine *mach,
		    struct r5sim_core *core,
		    const r5_inst *__inst)
{
	const r5_inst_u *inst = (const r5_inst_u *)__inst;
	u32 *inst_u32 = (u32 *)__inst;

	__set_reg(core, inst->rd, *inst_u32 & 0xfffff000);

	r5sim_itrace("LUI    %-3s <- 0x%08x\n",
		     r5sim_reg_to_str(inst->rd),
		     *inst_u32 & 0xfffff000);

	return 0;
}

static int exec_system(struct r5sim_machine *mach,
		       struct r5sim_core *core,
		       const r5_inst *__inst)
{
	const r5_inst_i *inst = (const r5_inst_i *)__inst;
	const u32 csr = inst->imm_11_0;
	int ret = 0;

	switch (inst->func3) {
	case 0x0: /* ECALL/EBREAK/etc. */
		switch (csr) {
		case 0x0: /* ECALL */
			if (inst->rs1 || inst->rd) {
				ret = TRAP_ILLEGAL_INST;
				goto done;
			}

			switch (core->priv) {
			case RV_PRIV_M:
				ret = TRAP_ECALL_MMODE;
				break;
			case RV_PRIV_S:
				ret = TRAP_ECALL_SMODE;
				break;
			default:
				r5sim_assert(!"No U-Mode yet!");
			}
			break;
		case 0x1: /* EBREAK */
			/*
			 * No debug support yet; just die.
			 */
			r5sim_info("Sim bug: no debug support.\n");
			r5sim_assert(0);
			break;
		case 0x2:   /* URET */
			ret = TRAP_ILLEGAL_INST;
			goto done;
		case 0x102: /* SRET */
			ret = TRAP_SRET;
			/* SRET can only be called by S-Mode. */
			if (core->priv != RV_PRIV_S) {
				ret = TRAP_ILLEGAL_INST;
				goto done;
			}
			break;
		case 0x302: /* MRET */
			ret = TRAP_MRET;
			/* MRET can only be called by M-Mode. */
			if (core->priv != RV_PRIV_M) {
				ret = TRAP_ILLEGAL_INST;
				goto done;
			}
			break;
		case 0x105: /* WFI */
			r5sim_core_wfi(core);
			break;
		}
		break;
	case 0x1: /* CSRRW */
		if (__csr_w(core, inst->rd, __get_reg(core, inst->rs1), csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	case 0x2: /* CSRRS */
		if (__csr_s(core, inst->rd, __get_reg(core, inst->rs1), csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	case 0x3: /* CSRRC */
		if (__csr_c(core, inst->rd, __get_reg(core, inst->rs1), csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	case 0x5: /* CSRRWI */
		if (__csr_w(core, inst->rd, inst->rs1, csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	case 0x6: /* CSRRSI */
		if (__csr_s(core, inst->rd, inst->rs1, csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	case 0x7: /* CSRRCI */
		if (__csr_c(core, inst->rd, inst->rs1, csr)) {
			ret = TRAP_ILLEGAL_INST;
			goto done;
		}
		break;
	default:
		ret = TRAP_ILLEGAL_INST;

		/* Skip tracing for illegal instructions. */
		goto done;
	}

	r5sim_itrace("%-6s   0x%3X rd=%-3s %s=%u\n",
		     r5sim_system_func3_to_str(inst->func3, csr),
		     csr,
		     r5sim_reg_to_str(inst->rd),
		     inst->func3 >= 0x5 ? "imm" : "rs",
		     inst->rs1);

done:
	return ret;
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

static struct r5_op_family *simple_core_opcode_fam(r5_inst *inst)
{
	u32 type_bits = (inst->opcode & 0x7c) >> 2;

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
	u32 inst_mem;
	struct r5_op_family *fam;
	u32 op_type;
	r5_inst *inst;
	int strap;

	/*
	 * the PC should always be 4 byte aligned since we check to make
	 * sure this happens during control flow instructions. But just
	 * double check.
	 */
	r5sim_assert(mach->memload32(mach, core->pc, &inst_mem));

	inst = (r5_inst *)(&inst_mem);
	fam = simple_core_opcode_fam(inst);
	op_type = (inst->opcode & 0x7c) >> 2;

	r5sim_itrace("PC 0x%08x i=0x%08x op=%-3d %-8s | ",
		     core->pc, inst_mem,
		     op_type, fam->op_name);

	if (fam->op_name == NULL || fam->op_exec == NULL) {
		return TRAP_ILLEGAL_INST;
	}

	if ((inst_mem & 0x3) != 0x3) {
		return TRAP_ILLEGAL_INST;
	}

	strap = fam->op_exec(mach, core, inst);
	if (strap)
		return strap;

	if (fam->incr_pc)
		core->pc += 4;

	return TRAP_ALL_GOOD;
}

struct r5sim_core *r5sim_simple_core_instance(
	struct r5sim_machine *mach)
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
