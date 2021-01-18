/*
 * Basic CPU core interfaces.
 */

#include <stdlib.h>

#include <time.h>

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/core.h>

/*
 * Timer callback so that the CSR timer function can get the host system
 * time. This generates a time in nanoseconds which is stored into the
 * TIME and TIMEH CSRs.
 *
 * The subsequent reads will then pull out this computed value from the
 * CSR file.
 */
static void
r5sim_csr_time(struct r5sim_core *core,
	       struct r5sim_csr *csr)
{
	long nsecs;
	time_t secs;
	uint64_t delta_ns;
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	nsecs = now.tv_nsec - core->start.tv_nsec;
	secs = now.tv_sec - core->start.tv_sec;

	if (nsecs < 0) {
		secs = secs - 1;
		nsecs += 1000000000;
	}

	delta_ns = secs * 1000000000 + nsecs;

	__raw_csr_write(&core->csr_file[CSR_TIME], (uint32_t)delta_ns);
	__raw_csr_write(&core->csr_file[CSR_TIMEH], (uint32_t)(delta_ns >> 32));
}

/*
 * To keep things simple for now each cycle == 1 instruction. This could
 * be changed in the future, but for now it seems like a viable enough
 * approach.
 */
static void
r5sim_core_incr(struct r5sim_core *core)
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
void
r5sim_core_exec(struct r5sim_machine *mach,
		struct r5sim_core *core,
		uint32_t pc)
{
	core->pc = pc;

	r5sim_info("Execution begins @ 0x%08x\n", pc);

	while (core->exec_one(mach, core) == 0)
		r5sim_core_incr(core);

	r5sim_info("HALT.\n");

	return;
}

void
__r5sim_core_add_csr(struct r5sim_core *core,
		     struct r5sim_csr *csr_reg,
		     uint32_t csr)
{
	r5sim_assert(csr < 4096);

	core->csr_file[csr] = *csr_reg;
}

static void
r5sim_core_default_csrs(struct r5sim_core *core)
{
	r5sim_core_add_csr(core, CSR_CYCLE,	0x0, CSR_READ);
	r5sim_core_add_csr(core, CSR_INSTRET,	0x0, CSR_READ);
	r5sim_core_add_csr(core, CSR_CYCLEH,	0x0, CSR_READ);
	r5sim_core_add_csr(core, CSR_INSTRET,	0x0, CSR_READ);

	r5sim_core_add_csr_fn(core, CSR_TIME,	0x0, CSR_READ, r5sim_csr_time, NULL);
	r5sim_core_add_csr_fn(core, CSR_TIMEH,	0x0, CSR_READ, r5sim_csr_time, NULL);
}

void
r5sim_core_init_common(struct r5sim_core *core)
{
	r5sim_core_default_csrs(core);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &core->start);
}

/*
 * All CSR instuctions read the CSR into rd; it's really only after that
 * the instructions have different effects.
 *
 * This will return NULL if the CSR is not implemented. Otherwise it
 * returns the address of ther CSR struct in the CSR file.
 */
static struct r5sim_csr *
__csr_always(struct r5sim_core *core, uint32_t rd, uint32_t csr)
{
	struct r5sim_csr *csr_reg;

	r5sim_assert(csr < 4096);

	csr_reg = &core->csr_file[csr];

	if ((csr_reg->flags & CSR_PRESENT) == 0)
		return NULL;

	if ((csr_reg->flags & CSR_READ) != 0 && rd != 0) {
		if (csr_reg->read_fn)
			csr_reg->read_fn(core, csr_reg);
		__set_reg(core, rd, __raw_csr_read(csr_reg));
	}

	return csr_reg;
}

void
__csr_w(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_write(csr_reg, value);
	}
}

void
__csr_s(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_set_mask(csr_reg, value);
	}
}

void
__csr_c(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_clear_mask(csr_reg, value);
	}
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
r5sim_op_i_func3_to_str(uint32_t func3, uint32_t func7)
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
r5sim_op_m_func3_to_str(uint32_t func3)
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

const char *
r5sim_system_func3_to_str(uint32_t func3)
{
	switch (func3) {
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
