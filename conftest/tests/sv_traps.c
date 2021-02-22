/*
 * Test supervisor traps; test both that M-Mode can receive
 * traps from S-Mode and M-Mode delegating the traps to S-Mode
 * works.
 */

#include "csr.h"
#include "conftest.h"

#include <r5sim/hw/vsys.h>

#define VSYS_BASE	0x4000100

static int ct_test_sv_trap_init(void *data)
{
	u32 stvec = (u32)&__trap_vector_sv;
	u32 sstatus, sie;

	/*
	 * Ask M-Mode to delegate exceptions to us.
	 */
	delegate_smode = 1;
	expect_exception = 1;
	asm volatile("ecall\n\t");

	/*
	 * Configure interrupt registers for S-Mode.
	 */
	stvec <<= 2;

	write_csr(CSR_STVEC, stvec);
	read_csr(CSR_STVEC, stvec);

	/*
	 * Enable interrupts and unmask the MSW interrupt.
	 */
	read_csr(CSR_SSTATUS, sstatus);
	read_csr(CSR_SIE,     sie);

	/*
	printf("STVEC:   0x%08x\n", stvec);
	printf("Starting regs:\n");
	printf("  SSTATUS: 0x%08x\n", sstatus);
	printf("  SIE:     0x%08x\n", sie);
	*/

	set_field(sstatus, CSR_SSTATUS_SIE, 1);

	set_field(sie,     CSR_SIE_SSIE,    1);
	set_field(sie,     CSR_SIE_STIE,    1);

	write_csr(CSR_SSTATUS, sstatus);
	write_csr(CSR_SIE,     sie);

	read_csr(CSR_SSTATUS, sstatus);
	read_csr(CSR_SIE,     sie);

	/*
	printf("Updated regs:\n");
	printf("  SSTATUS: 0x%08x\n", sstatus);
	printf("  SIE:     0x%08x\n", sie);
	*/

	return 1;
}

static int ct_test_sv_access_align(void *data)
{
	u32 start, end;
	volatile u32 value;
	u32 target = 0x20000001;
	volatile u32 *addr = (u32 *)target;

	expect_exception = 1;
	s_excep_exec = 0;

	read_csr(CSR_CYCLE, start);
	__asm__ volatile("lw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (addr));
	read_csr(CSR_CYCLE, end);

	(void) value;

	if (!s_excep_exec)
		return 0;

	return (end - start) > 62;
}

static int ct_test_sv_illegal_inst(void *data)
{
	u32 start, end;

	expect_exception = 1;
	s_excep_exec = 0;

	read_csr(CSR_CYCLE, start);
	asm volatile(".long 0x00000000\n\t");
	read_csr(CSR_CYCLE, end);

	if (!s_excep_exec)
		return 0;

	return (end - start) > 62;
}

static int ct_test_sv_ecall(void *data)
{
	u32 start, end;

	expect_exception = 1;
	s_excep_exec = 0;

	read_csr(CSR_CYCLE, start);
	asm volatile("ecall\n\t");
	read_csr(CSR_CYCLE, end);

	if (!s_excep_exec)
		return 0;

	return (end - start) > 62;
}

static int ct_test_sv_timer_intr(void *data)
{
	u32 start, end;
	u32 config = 0;

	set_field(config,
		  VSYS_TIMER_CONFIG_PRECISION,
		  VSYS_TIMER_CONFIG_PRECISION_MSECS);
	set_field(config,
		  VSYS_TIMER_CONFIG_ACTIVATE,
		  VSYS_TIMER_CONFIG_ACTIVATE_TRIGGER);

	/*
	 * Configure and trigger the timer.
	 */
	writel(VSYS_BASE + VSYS_TIMER_INTERVAL, 10);
	writel(VSYS_BASE + VSYS_TIMER_CONFIG,   config);

	s_intr_exec = 0;

	read_csr(CSR_CYCLE, start);

	__asm__ volatile("wfi\n\t");

	read_csr(CSR_CYCLE, end);

	if (!s_intr_exec)
		return 0;

	return (end - start) > 62;
}

static int ct_test_sv_sw_intr(void *data)
{
	u32 start, end;

	s_intr_exec = 0;

	read_csr(CSR_CYCLE, start);

	writel(VSYS_BASE + VSYS_M_SW_INTERRUPT, 1);

	/*
	 * The above write should triger an interrupt this cycle.
	 * That interrupt will involve some number of instructions
	 * greater than 1. The number of instructions is at least
	 * 62 for a successful trap since it saves/restores all
	 * registers (except x0).
	 *
	 * So if end - start > 62 then we should have definitely
	 * executed the trap.
	 */
	read_csr(CSR_CYCLE, end);

	if (!s_intr_exec)
		return 0;

	return (end - start) > 62;
}

int ct_test_access_align(void *data);
int ct_test_illegal_inst(void *data);
int ct_test_ecall(void *data);
int ct_test_timer_intr(void *data);
int ct_test_sw_intr(void *data);

static const struct ct_test op_sv_traps[] = {
	/*
	 * Rerun the M-mode interrupt tests; they should just work.
	 */
	CT_TEST(ct_test_access_align,		NULL,			"access_align"),
	CT_TEST(ct_test_illegal_inst,		NULL,			"illegal_inst"),
	CT_TEST(ct_test_ecall,			NULL,			"ecall"),
	CT_TEST(ct_test_sw_intr,		NULL,			"sw_intr"),
	CT_TEST(ct_test_timer_intr,		NULL,			"timer_intr"),

	CT_TEST(ct_test_sv_trap_init,		NULL,			"init"),

	CT_TEST(ct_test_sv_access_align,	NULL,			"access_align_sv"),
	CT_TEST(ct_test_sv_illegal_inst,	NULL,			"illegal_inst_sv"),
	CT_TEST(ct_test_sv_ecall,		NULL,			"ecall_sv"),
	CT_TEST(ct_test_sv_sw_intr,		NULL,			"sw_intr_sv"),
	CT_TEST(ct_test_sv_timer_intr,		NULL,			"timer_intr_sv"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *ct_sv_traps(void)
{
	return op_sv_traps;
}
