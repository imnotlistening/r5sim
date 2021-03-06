/*
 * Basic CPU core interfaces.
 */

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <r5sim/log.h>
#include <r5sim/app.h>
#include <r5sim/env.h>
#include <r5sim/isa.h>
#include <r5sim/core.h>
#include <r5sim/trap.h>
#include <r5sim/util.h>
#include <r5sim/machine.h>

/*
 * Check the medeleg register to see what priv level we should take
 * this exception at.
 */
static u32 __exception_priv_target(struct r5sim_core *core, u32 code)
{
	u32 code_bit = (1 << code);

	/*
	 * If the exception is delegated then we'll take it at supervisor.
	 * otherwise it's always taken at machine.
	 */
	if (core->medeleg & code_bit)
		return RV_PRIV_S;

	return RV_PRIV_M;
}

/*
 * Depending on priv level, we need to load either M CSRs or S
 * CSRs.
 */
static void __load_trap_regs(struct r5sim_core *core,
			     u32 priv, u32 code, u32 intr)
{
	u32 cause = 0;
	u32 prev_intr;

	if (priv == RV_PRIV_M) {
		/*
		 * Set previous priv to the current priv. The core's priv
		 * level will be set for the target before executing the trap.
		 */
		prev_intr = get_field(core->mstatus, CSR_MSTATUS_MIE);

		set_field(core->mstatus, CSR_MSTATUS_MIE,  0);
		set_field(core->mstatus, CSR_MSTATUS_MPIE, prev_intr);
		set_field(core->mstatus, CSR_MSTATUS_MPP,  core->priv);

		set_field(cause, CSR_MCAUSE_CODE,      code);
		set_field(cause, CSR_MCAUSE_INTERRUPT, intr);

		/* Save PC to MEPC and load MCASUE. SW will need this. */
		csr_write(core, CSR_MEPC,   core->pc);
		csr_write(core, CSR_MCAUSE, cause);
	} else if (priv == RV_PRIV_S) {
		/* Traps down in privilege level are not allowed. */
		r5sim_assert(core->priv <= RV_PRIV_S);

		prev_intr = get_field(core->mstatus, CSR_MSTATUS_SIE);

		set_field(core->mstatus, CSR_MSTATUS_SIE,  0);
		set_field(core->mstatus, CSR_MSTATUS_SPIE, prev_intr);
		set_field(core->mstatus, CSR_MSTATUS_SPP,  core->priv);

		set_field(cause, CSR_SCAUSE_CODE,      code);
		set_field(cause, CSR_SCAUSE_INTERRUPT, intr);

		csr_write(core, CSR_SEPC,   core->pc);
		csr_write(core, CSR_SCAUSE, cause);
	} else {
		r5sim_assert(0);
	}
}

/*
 * To execute a trap we must:
 *
 * First we'll need to figure what priv level we are going to jump
 * to. All interrupts and exceptions, by default, are executed in
 * M-mode. However, based on the m[ie]deleg registers, we may end
 * up executing the trap or interrupt in a lower priv level.
 *
 * Regardless, we'll end up setting a few registers:
 *
 *  xEPC
 *  xCAUSE
 *  xSTATUS
 *
 * Save the current PC to the xEPC CSR based on current priviledge
 * mode and save the code to MCAUSE. Currently there's no support
 * for anything except MMode so we can forgo worrying about the
 * priv level.
 *
 * Load and compute the interrupt vector base. Note that this
 * assumes the interrupt setting will have only one bit ever set
 * at a time. This may not be viable, long term.
 * Set the priviledge mode to MODE_MACHINE. We don't do anything
 * here at the moment sicne we only support mmode. Eventually we'll
 * have S mode and will need to work out delegation considerations
 * for priv levels and stuff. For now, this is a no-op.
 *
 * Begin executing instructions at the computed interrupt vector.
 *
 * Reload PC with xEPC.
 *
 * Once this function returns the regular execution stream resumes.
 */
void __r5sim_core_push_trap(struct r5sim_core *core,
			    u32 priv, u32 code, u32 intr)
{
	u32 vec, base, mode;
	u32 cause = 0;

	__load_trap_regs(core, priv, code, intr);

	vec   = csr_read(core, priv == RV_PRIV_M ? CSR_MTVEC : CSR_STVEC);
	cause = csr_read(core, priv == RV_PRIV_M ? CSR_MCAUSE : CSR_SCAUSE);

	/*
	 * Compute trap vector; it's always just base for synchronous
	 * exceptions. But for interrupts we have to check the VECTORED
	 * field.
	 */
	base  = get_field(vec, CSR_MTVEC_BASE);
	mode  = get_field(vec, CSR_MTVEC_MODE);

	if (intr && mode == CSR_MTVEC_MODE_VECTORED)
		base += (cause << 2);

	r5sim_dbg("Push trap @ PC=0x%08x.\n", core->pc);
	r5sim_dbg("  Current priv: 0x%x\n", core->priv);
	r5sim_dbg("  Target  priv: 0x%x\n", priv);
	r5sim_dbg("  mstatus:      0x%08x\n", core->mstatus);
	r5sim_dbg("  mie:          0x%08x\n", core->mie);
	r5sim_dbg("  mip:          0x%08x\n", core->mip);
	r5sim_dbg("  mideleg:      0x%08x\n", core->mideleg);
	r5sim_dbg("  medeleg:      0x%08x\n", core->medeleg);
	r5sim_dbg("  MCAUSE: 0x%08x\n", cause);
	r5sim_dbg("    intr:  %s\n", intr ? "yes" : "no");
	r5sim_dbg("  MTVEC:  0x%08x\n", vec);
	r5sim_dbg("    Base: 0x%08x\n", base);
	r5sim_dbg("    Mode: %s\n", mode ? "vectored" : "direct");

	core->priv = priv;
	core->pc = base;
}

static void r5sim_core_push_exception(struct r5sim_core *core,
				     u32 code)
{
	u32 priv = __exception_priv_target(core, code);

	__r5sim_core_push_trap(core, priv, code, 0);
}

static void r5sim_core_pop_trap_m(struct r5sim_core *core)
{
	u32 prev_priv = get_field(core->mstatus, CSR_MSTATUS_MPP);
	u32 prev_ie   = get_field(core->mstatus, CSR_MSTATUS_MPIE);

	r5sim_dbg("Pop trap-M!\n");

	r5sim_assert(core->priv == RV_PRIV_M);

	set_field(core->mstatus, CSR_MSTATUS_MIE,  prev_ie);
	set_field(core->mstatus, CSR_MSTATUS_MPIE, 1);
	set_field(core->mstatus, CSR_MSTATUS_MPP,  RV_PRIV_M);

	core->priv = prev_priv;
	core->pc = csr_read(core, CSR_MEPC);
}

static void r5sim_core_pop_trap_s(struct r5sim_core *core)
{
	u32 prev_priv = get_field(core->mstatus, CSR_MSTATUS_SPP);
	u32 prev_ie   = get_field(core->mstatus, CSR_MSTATUS_SPIE);

	r5sim_dbg("Pop trap-S!\n");

	r5sim_assert(core->priv == RV_PRIV_S);

	set_field(core->mstatus, CSR_MSTATUS_SIE,  prev_ie);
	set_field(core->mstatus, CSR_MSTATUS_SPIE, 1);
	set_field(core->mstatus, CSR_MSTATUS_SPP,  RV_PRIV_U);

	core->priv = prev_priv;
	core->pc = csr_read(core, CSR_SEPC);
}

/*
 * To keep things simple for now each cycle == 1 instruction. This could
 * be changed in the future, but for now it seems like a viable enough
 * approach.
 */
void r5sim_core_incr(struct r5sim_core *core)
{
	core->csr_file[CSR_CYCLE].value += 1;
	core->csr_file[CSR_INSTRET].value += 1;

	/* Handle wrap: if value is 0, then it must have wrapped. */
	if (!core->csr_file[CSR_CYCLE].value)
		core->csr_file[CSR_CYCLEH].value += 1;
	if (!core->csr_file[CSR_INSTRET].value)
		core->csr_file[CSR_INSTRETH].value += 1;
}

/*
 * Start execution on a RISC-V core!
 *
 * When core->exec_one() returns non-zero, HALT the machine.
 */
void r5sim_core_exec(struct r5sim_machine *mach,
		     struct r5sim_core *core,
		     u32 nr)
{
	u32 done = 0;

	r5sim_dbg("EXEC @ 0x%08x (%x)\n", core->pc, core->priv);

	while (1) {
		int trap;

		if (mach->debug && !mach->step)
			return;

		trap = core->exec_one(mach, core);

		/*
		 * Check if we should push an interrupt. If so we'll
		 * push the interrupt (which adjusts the PC) and then
		 * just continue on.
		 *
		 * Interrupts supercede exceptions. So we do this first.
		 */
		if (r5sim_core_handle_intr(core))
			goto inst_done;

		/*
		 * No interrupt so handle possible exceptions; here we
		 * handle either no exception or an xRET instruction.
		 */
		switch (trap) {
		case TRAP_ALL_GOOD:
			r5sim_core_incr(core);
			goto inst_done;
		case TRAP_MRET:
			r5sim_core_incr(core);
			r5sim_core_pop_trap_m(core);
			goto inst_done;
		case TRAP_SRET:
			r5sim_core_incr(core);
			r5sim_core_pop_trap_s(core);
			goto inst_done;
		case TRAP_BREAK_POINT:
			kill(getpid(), SIGTSTP);
			/*
			 * Make sure that debug is really set; then we'll
			 * continue and finally return. If we don't wait we
			 * may end up executing a few extra instructions
			 * before the debugger wakes up and sets mach->debug
			 * to true.
			 *
			 * It's possible we hit a break point while already
			 * in debug mode (e.g stepping). That should be
			 * perfectly fine.
			 */
			while (!mach->debug)
				;
			return;
		default:
			/*
			 * An actual exception.
			 */
			r5sim_assert(trap >= 0);

			/*
			 * If we are in M-mode we won't take the exception
			 * if it's been delegated to supervisor.
			 */
			if (core->priv == RV_PRIV_M &&
			    ((1 << trap) & core->medeleg))
				goto inst_done;

			/* Otherwise we can take this trap. */
			r5sim_core_push_exception(core, (u32)trap);
		}

	inst_done:
		done += 1;

		/*
		 * If the debugger asks us to run nr instructions, return when
		 * we hit that number.
		 */
		if (nr && done >= nr)
			return;
	}
}

void r5sim_core_init_common(struct r5sim_core *core)
{
	r5sim_core_default_csrs(core);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &core->start);

	/*
	 * Start off in M-mode.
	 */
	core->priv = RV_PRIV_M;

	/*
	 * Default MMU implementation; has a PMP and will eventually have
	 * an page table.
	 */
	r5sim_mmu_use_default(core);

	/* Use tracing? */
	core->itrace = r5sim_app_get_args()->itrace;
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
			return "EBREAK";
		case 0x2:   /* URET - not supported quite yet. */
			return "ERR";
		case 0x102: /* SRET */
			return "SRET";
		case 0x302: /* MRET */
			return "MRET";
		case 0x105: /* WFI */
			return "WFI";
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
