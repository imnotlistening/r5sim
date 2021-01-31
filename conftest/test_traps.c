/*
 * Simple tests to make sure that what traps we can test without
 * an MMU are mostly working.
 *
 * This basically just issues a bad instruction and then expects
 * the trap to skip said instruction.
 */

#include "csr.h"
#include "conftest.h"

#include <r5sim/hw/vsys.h>

#define VSYS_BASE	0x4000100

static int ct_test_access_align(void *data)
{
	u32 something = 5, another = 2;
	u32 *addr = &something;
	char *addr_tmp;

	addr_tmp = (char *)addr;
	addr_tmp += 1;
	addr = (u32 *)addr_tmp;

	another = *addr;

	return something != another;
}

static int ct_test_illegal_inst(void *data)
{
	u32 start, end;

	read_csr(CSR_CYCLE, start);
	asm volatile(".long 0x00000000\n\t");
	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

static int ct_test_ecall(void *data)
{
	u32 start, end;

	read_csr(CSR_CYCLE, start);
	asm volatile("ecall\n\t");
	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

static int ct_test_timer_intr(void *data)
{
	u32 start, end;
	u32 config = 0;

	set_field(config,
		  VSYS_TIMER_CONFIG_PRECISION,
		  VSYS_TIMER_CONFIG_PRECISION_MSECS);
	set_field(config,
		  VSYS_TIMER_CONFIG_ACTIVATE,
		  VSYS_TIMER_CONFIG_ACTIVATE_TRIGGER);

	read_csr(CSR_CYCLE, start);

	/*
	 * Configure and trigger the timer.
	 */
	writel(VSYS_BASE + VSYS_TIMER_INTERVAL, 10);
	writel(VSYS_BASE + VSYS_TIMER_CONFIG,   config);

	__asm__ volatile("wfi\n\t");

	read_csr(CSR_CYCLE, end);

	return (end - start) > 62;
}

static int ct_test_sw_intr(void *data)
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

static const struct ct_test op_traps[] = {
	CT_TEST(ct_test_access_align,		NULL,			"access_align"),
	CT_TEST(ct_test_illegal_inst,		NULL,			"illegal_inst"),
	CT_TEST(ct_test_ecall,			NULL,			"ecall"),
	CT_TEST(ct_test_sw_intr,		NULL,			"sw_intr"),
	CT_TEST(ct_test_timer_intr,		NULL,			"timer_intr"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *ct_traps(void)
{
	return op_traps;
}
