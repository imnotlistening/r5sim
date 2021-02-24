/*
 * Test code - let's see if we can make a simple test to keep our
 * r5 simple_core mostly functional.
 */

#include <ct/csr.h>
#include <ct/time.h>
#include <ct/tests.h>
#include <ct/symbols.h>
#include <ct/conftest.h>

static ct_test_list_fn submodules[] = {
	ct_env,
	ct_system,
	ct_load_store,
	ct_muldiv,
	ct_op,
	ct_traps,
	NULL
};

static ct_test_list_fn submodules_sv[] = {
	ct_sv_traps,
	NULL
};

static void ct_jump_to_smode(void)
{
	u32 mstatus;

	read_csr(CSR_MSTATUS, mstatus);
	set_field(mstatus, CSR_MSTATUS_MPP, 0x1); /* S-Mode */

	write_csr(CSR_MSTATUS, mstatus);
	write_csr(CSR_MEPC,    (u32)&_smode_start);
	write_csr(CSR_MIE,     0xaa);

	__asm__("mret\n\t");
}

void start(void)
{
	printf("Welcome to a super simple R5 conf test\n");

	ct_sys_info();
	trap_setup();

	ct_compute_perf();

	ct_exec_tests(submodules);

	printf("Backtrace test:\n");
	backtrace_test();
	printf("\n\n");

	printf("\n\nJumping to S-Mode\n");
	ct_jump_to_smode();
}

/*
 * We'll get here once we are executing in supervisor mode. We don't mess with
 * memory prot stuff or anything so we are executing in the same physical
 * address space as the previous M-Mode code.
 *
 * Use this for testing the M-Mode and S-Mode interactions, traps, interrupts,
 * etc.
 */
void sv_start(void)
{
	printf("-- Executing Supervisor mode tests! --\n");

	ct_exec_tests(submodules_sv);

	while (1);
}
