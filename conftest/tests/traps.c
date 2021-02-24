/*
 * Simple tests to make sure that what traps we can test without
 * an MMU are mostly working.
 *
 * This basically just issues a bad instruction and then expects
 * the trap to skip said instruction.
 */

#include <ct/csr.h>
#include <ct/conftest.h>
#include <ct/tests.h>

#include <r5sim/hw/vsys.h>

#define VSYS_BASE	0x4000100

int ct_test_access_align(void *data)
{
	u32 start, end;
	volatile u32 value;
	u32 target = 0x20000001;
	volatile u32 *addr = (u32 *)target;

	expect_exception = 1;

	read_csr(CSR_CYCLE, start);
	__asm__ volatile("lw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (addr));
	read_csr(CSR_CYCLE, end);

	(void) value;

	return (end - start) > 62;
}

int ct_test_illegal_inst(void *data)
{
	u32 start, end;

	expect_exception = 1;

	read_csr(CSR_CYCLE, start);
	asm volatile(".long 0x00000000\n\t");
	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

int ct_test_ecall(void *data)
{
	u32 start, end;

	expect_exception = 1;

	read_csr(CSR_CYCLE, start);
	asm volatile("ecall\n\t");
	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

int ct_test_timer_intr(void *data)
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

	read_csr(CSR_CYCLE, start);

	__asm__ volatile("wfi\n\t");

	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

int ct_test_sw_intr(void *data)
{
	u32 start, end;

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

	return (end - start) > 62;
}

/*
 * Delegate all interrupts (just for testing) to s-mode; this means we
 * should _not_ take these interrupts, even if pending, in M-mode.
 */
static int ct_test_deleg_intr(void *data)
{
	write_csr(CSR_MIDELEG, 0xffffffff);

	return 1;
}

static int ct_test_deleg_done_intr(void *data)
{
	write_csr(CSR_MIDELEG, 0x0);

	return 1;
}

int ct_test_sw_intr_deleg(void *data)
{
	u32 start, end;
	u32 mip;

	read_csr(CSR_CYCLE, start);

	writel(VSYS_BASE + VSYS_M_SW_INTERRUPT, 1);

	/*
	 * This time, since the interrupt is delegated, we shouldn't get
	 * interrupted. Thus the instruction could should be well under
	 * 62.
	 */
	read_csr(CSR_CYCLE, end);

	/*
	 * Also, let's verify that MIP shows the timer interrupt present.
	 */
	read_csr(CSR_MIP, mip);

	if (!get_field(mip, CSR_MIP_MSIP))
		return 0;

	clear_csr(CSR_MIP, mip);

	return (end - start) < 10;
}

static int ct_test_timer_intr_deleg(void *data)
{
	u32 start, end;
	u32 config = 0;
	u32 mip;

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

	read_csr(CSR_CYCLE, start);

	__asm__ volatile("wfi\n\t");

	read_csr(CSR_CYCLE, end);

	read_csr(CSR_MIP, mip);
	if (!get_field(mip, CSR_MIP_MTIP))
		return 0;

	clear_csr(CSR_MIP, mip);

	return (end - start) < 10;
}

static const struct ct_test op_traps[] = {
	CT_TEST(ct_test_access_align,		NULL,			"access_align"),
	CT_TEST(ct_test_illegal_inst,		NULL,			"illegal_inst"),
	CT_TEST(ct_test_ecall,			NULL,			"ecall"),
	CT_TEST(ct_test_sw_intr,		NULL,			"sw_intr"),
	CT_TEST(ct_test_timer_intr,		NULL,			"timer_intr"),
	CT_TEST(ct_test_deleg_intr,		NULL,			"deleg_intr"),
	CT_TEST(ct_test_sw_intr_deleg,		NULL,			"sw_intr_deleg"),
	CT_TEST(ct_test_timer_intr_deleg,	NULL,			"timer_intr_deleg"),
	CT_TEST(ct_test_deleg_done_intr,	NULL,			"deleg_done_intr"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *ct_traps(void)
{
	return op_traps;
}
