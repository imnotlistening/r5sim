/*
 * Basic CPU core interfaces.
 */

#include <stdlib.h>

#include <time.h>

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/isa.h>
#include <r5sim/core.h>
#include <r5sim/trap.h>
#include <r5sim/util.h>

/*
 * To execute a trap we must:
 *
 *   Save the current PC to the xEPC CSR based on current priviledge
 *   mode and save the code to MCAUSE. Currently there's no support
 *   for anything except MMode so we can forgo worrying about the
 *   priv level.
 *
 *   Load and compute the interrupt vector base. Note that this
 *   assumes the interrupt setting will have only one bit ever set
 *   at a time. This may not be viable, long term.

 *   Set the priviledge mode to MODE_MACHINE. We don't do anything
 *   here at the moment sicne we only support mmode. Eventually we'll
 *   have S mode and will need to work out delegation considerations
 *   for priv levels and stuff. For now, this is a no-op.
 *
 *   Begin executing instructions at the computed interrupt vector.
 *
 *   Reload PC with xEPC.
 *
 * Once this function returns the regular execution stream resumes.
 */
static int __r5sim_core_trap(struct r5sim_machine *mach,
			     struct r5sim_core *core,
			     u32 code, u32 intr)
{
	u32 mtvec, base, mode;
	u32 mcause = 0;

	/*
	 * First - check the depth and see if we should teminate. We
	 * don't want infinite traps. Otherwise increment our trap depth
	 * and proceed.
	 */
	if (core->trap_depth >= R5SIM_TRAP_DEPTH_MAX)
		return -1;
	core->trap_depth += 1;

	set_field(mcause, CSR_MCAUSE_CODE,      code);
	set_field(mcause, CSR_MCAUSE_INTERRUPT, intr);

	/* Save PC to MEPC and load MCASUE. SW will need this. */
	csr_write(core, CSR_MEPC,   core->pc);
	csr_write(core, CSR_MCAUSE, mcause);

	mtvec = csr_read(core, CSR_MTVEC);

	/*
	 * Compute trap vector; it's always just base for synchronous
	 * exceptions. But for interrupts we have to check the VECTORED
	 * field. Currently we assume that mcause will only have one bit
	 * set for interrupts... Hmm.
	 */
	base  = get_field(mtvec, CSR_MTVEC_BASE);
	mode  = get_field(mtvec, CSR_MTVEC_MODE);

	if (intr && mode == CSR_MTVEC_MODE_VECTORED)
		base += (mcause << 2);

	r5sim_dbg("Trap detected @ PC=0x%08x.\n", core->pc);
	r5sim_dbg("  MCAUSE: 0x%08x\n", mcause);
	r5sim_dbg("    intr:  %s\n", intr ? "yes" : "no");
	r5sim_dbg("    depth: %d\n", core->trap_depth);
	r5sim_dbg("  MTVEC:  0x%08x\n", mtvec);
	r5sim_dbg("    Base: 0x%08x\n", base);
	r5sim_dbg("    Mode: %s\n", mode ? "vectored" : "direct");

	r5sim_core_exec(mach, core, base);

	r5sim_dbg("Trap returned.\n");

	core->pc = csr_read(core, CSR_MEPC);
	core->trap_depth -= 1;

	return 0;
}

static int r5sim_core_intr(struct r5sim_machine *mach,
			   struct r5sim_core *core)
{
	struct r5sim_core_interrupt *intr = r5sim_core_intr_next(core);
	u32 cause;

	r5sim_assert(intr);

	cause = intr->cause;
	free(intr);

	/*
	 * If the cause is unmasked, then we do an actual interrupt.
	 * Otherwise we'll just return.
	 */
	if (core->mie & (1 << cause))
		return __r5sim_core_trap(mach, core, cause, 1);

	return 0;
}

static int r5sim_core_sync_trap(struct r5sim_machine *mach,
				struct r5sim_core *core,
				u32 code)
{
	return __r5sim_core_trap(mach, core, code, 0);
}

/*
 * To keep things simple for now each cycle == 1 instruction. This could
 * be changed in the future, but for now it seems like a viable enough
 * approach.
 */
static void r5sim_core_incr(struct r5sim_core *core)
{
	core->csr_file[CSR_CYCLE].value += 1;
	core->csr_file[CSR_INSTRET].value += 1;

	/* Handle wrap: if value is 0, then it must have wrapped. */
	if (!core->csr_file[CSR_CYCLE].value)
		core->csr_file[CSR_CYCLEH].value += 1;
	if (!core->csr_file[CSR_INSTRET].value)
		core->csr_file[CSR_INSTRETH].value += 1;
}

static int __core_should_handle_intr(struct r5sim_core *core)
{
	/*
	 * If an interrupt signal is pending, then check to see if
	 * we have any unmasked interrupts. If we do, then we should
	 * handle the interrupt.
	 */
	return r5sim_core_intr_pending(core) &&
		get_field(core->mstatus, CSR_MSTATUS_MIE) &&
		(core->mie & core->mip);
}

/*
 * Start execution on a RISC-V core!
 *
 * When core->exec_one() returns non-zero, HALT the machine.
 */
void r5sim_core_exec(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     u32 pc)
{
	core->pc = pc;

	while (1) {
		int trap;

		/*
		 * Execute the primary instruction stream until we get
		 * an exception.
		 */
		while (1) {
			trap = core->exec_one(mach, core);

			/*
			 * Check if we should just continue on to the next
			 * instruction. If any of the following is true, then
			 * we'll break out of the main execution loop and
			 * service the trap or interrupt.
			 *
			 *   - A synchronous trap has been enountered;
			 *   - We have a pending interrupt signal _and_
			 *   - There's an unmasked interrupt bit set.
			 */
			if (!trap &&
			    !__core_should_handle_intr(core)) {
				r5sim_core_incr(core);
				continue;
			}

			/*
			 * A return from trap;
			 */
			if (trap == TRAP_MRET) {
				r5sim_core_incr(core);
				return;
			}

			/*
			 * Otherwise if there's a trap we'll definitely
			 * need to break. However, for an interrupt we
			 * break _after_ the last executed instruction
			 * where as for a trap we are breaking _before_
			 * the instruction has effectively executed.
			 *
			 * Note, though, that really we test for there
			 * not being a trap; you could have both an
			 * exception and a trap at exactly the same time.
			 */
			if (!trap)
				r5sim_core_incr(core);

			break;
		}

		/*
		 * We broke from the above loop. That means we got either
		 * a trap or an interrupt (or both). Handle it.
		 *
		 * If we fail to handle the exception then just HALT for
		 * now. No nested trap handling.
		 *
		 * Ugh. Ok. We'll prioritize interrupts over traps but we
		 * have to make sure we don't take an interrupt if
		 * interrupts are globally disabled or there's no unmasked
		 * interrupts pending.
		 */
		if (__core_should_handle_intr(core)) {
			if (r5sim_core_intr(mach, core))
				break;
		} else {
			r5sim_assert(trap);
			if (r5sim_core_sync_trap(mach, core, (u32)trap))
				break;
		}

		/*
		 * Otherwise, once we are done with the trap, resume
		 * normal execution.
		 */
	}

	return;
}

void r5sim_core_init_common(struct r5sim_core *core)
{
	pthread_mutex_init(&core->intr_lock, NULL);
	INIT_LIST_HEAD(&core->intrs);

	r5sim_core_default_csrs(core);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &core->start);
}

void r5sim_core_describe(struct r5sim_core *core)
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

static const char *reg_abi_names[32] = {
	/* 0  */ " z",  "ra",  "sp",  "gp",  "tp",  "t0",  "t1",  "t2",
	/* 8  */ "fp",  "s1",  "a0",  "a1",  "a2",  "a3",  "a4",  "a5",
	/* 16 */ "a6",  "a7",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
	/* 24 */ "s8",  "s9",  "s10", "s11", "t3",  "t4",  "t5",  "t6",
};

static const char *reg_names[32] = {
	/* 0  */ "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
	/* 8  */ "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
	/* 16 */ "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
	/* 24 */ "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",
};

const char *r5sim_reg_to_abi_str(u32 reg)
{
	return reg_abi_names[reg];
}

const char *r5sim_reg_to_str(u32 reg)
{
	return reg_names[reg];
}

const char *r5sim_load_func3_to_str(u32 func3)
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

const char *r5sim_store_func3_to_str(u32 func3)
{
	switch (func3) {
	case 0x0: /* SB */
		return "SB";
	case 0x1: /* SH */
		return "SH";
	case 0x2: /* SW */
		return "SW";
	}

	return "ERR";
}

const char *r5sim_op_imm_func3_to_str(u32 func3)
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

const char *r5sim_op_i_func3_to_str(u32 func3, u32 func7)
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

const char *r5sim_op_m_func3_to_str(u32 func3)
{
	switch (func3) {
	case 0x0: /* MUL */
		return "MUL";
	case 0x1: /* MULH */
		return "MULH";
	case 0x2: /* MULHSU */
		return "MULHSU";
	case 0x3: /* MULHU */
		return "MULHU";
	case 0x4: /* DIV */
		return "DIV";
	case 0x5: /* DIVU */
		return "DIVU";
	case 0x6: /* REM */
		return "REM";
	case 0x7: /* REMU */
		return "REMU";
	}

	return ERR;
}

const char *r5sim_branch_func3_to_str(u32 func3)
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

const char *r5sim_system_func3_to_str(u32 func3, u32 csr)
{
	switch (func3) {
	case 0x0: /* ECALL, MRET. */
		switch (csr) {
		case 0x0:   /* ECALL */
			return "ECALL";
		case 0x1:   /* EBREAK */
		case 0x2:   /* URET */
		case 0x102: /* SRET */
			return "ERR";
		case 0x302: /* MRET */
			return "MRET";
		}
	case 0x1: /* CSRRW */
		return "CSRRW";
	case 0x2: /* CSRRS */
		return "CSRRS";
	case 0x3: /* CSRRC */
		return "CSRRC";
	case 0x5: /* CSRRWI */
		return "CSRRWI";
	case 0x6: /* CSRRSI */
		return "CSRRSI";
	case 0x7: /* CSRRCI */
		return "CSRRCI";
	}

	return "ERR";
}
