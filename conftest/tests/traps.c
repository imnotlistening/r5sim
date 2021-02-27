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
#include <ct/trap_tests.h>

#include <r5sim/hw/vsys.h>

#define VSYS_BASE	0x4000100

void ct_test_ld_align(void)
{
	u32 value;
	u32 target = 0x20000001;

	__asm__ volatile("lw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (target));

	(void) value;
}

void ct_test_st_align(void)
{
	u32 value = 0xff00ff00;
	u32 target = 0x20000001;

	__asm__ volatile("sw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (target));

	(void) value;
}

void ct_test_inst_align(void)
{
	u32 target = 0x20000003;

	__asm__ volatile("jalr	%0\n\t"
			 :
			 : "r" (target));
}

void ct_test_ld_fault(void)
{
	u32 value;
	u32 target = 0x4;

	__asm__ volatile("lw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (target));

	(void) value;
}

void ct_test_st_fault(void)
{
	u32 value = 0xff00ff00;
	u32 target = 0x4;

	__asm__ volatile("sw	%1, 0(%0)\n\t"
			 : "=r" (value)
			 : "r" (target));

	(void) value;
}

void ct_test_illegal_inst(void)
{
	asm volatile(".long 0x00000000\n\t");
}


void ct_test_ecall(void)
{
	asm volatile("ecall\n\t");
}

int ct_test_exception(void *__test)
{
	u32 start, end;
	struct ct_excep_test *test = __test;

	/*
	 * Work with the trap API: set this variable to tell the
	 * trap code this next trap is expected.
	 */
	expect_exception = 1;

	/*
	 * Read the CYCLE csr so that we can verify that a trap
	 * actually happened.
	 *
	 * This relies on fault_func executing only a few instructions
	 * at most.
	 */
	read_csr(CSR_CYCLE, start);
	test->fault_func();
	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

struct ct_excep_test ld_align     = { ct_test_ld_align };
struct ct_excep_test st_align     = { ct_test_st_align };
struct ct_excep_test inst_align   = { ct_test_inst_align };
struct ct_excep_test ld_fault     = { ct_test_ld_fault };
struct ct_excep_test st_fault     = { ct_test_st_fault };
struct ct_excep_test illegal_inst = { ct_test_illegal_inst };
struct ct_excep_test ecall        = { ct_test_ecall };

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
	/*
	 * CPU exceptions.
	 */
	CT_TEST(ct_test_exception,		&ld_align,	"ld_align"),
	CT_TEST(ct_test_exception,		&ld_fault,	"ld_fault"),
	CT_TEST(ct_test_exception,		&st_align,	"st_align"),
	CT_TEST(ct_test_exception,		&st_fault,	"st_fault"),
	CT_TEST(ct_test_exception,		&inst_align,	"inst_align"),
	CT_TEST(ct_test_exception,		&illegal_inst,	"illegal_inst"),
	CT_TEST(ct_test_exception,		&ecall,		"ecall"),

	CT_TEST(ct_test_sw_intr,		NULL,		"sw_intr"),
	CT_TEST(ct_test_timer_intr,		NULL,		"timer_intr"),
	CT_TEST(ct_test_deleg_intr,		NULL,		"deleg_intr"),
	CT_TEST(ct_test_sw_intr_deleg,		NULL,		"sw_intr_deleg"),
	CT_TEST(ct_test_timer_intr_deleg,	NULL,		"timer_intr_deleg"),
	CT_TEST(ct_test_deleg_done_intr,	NULL,		"deleg_done_intr"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *ct_traps(void)
{
	return op_traps;
}
