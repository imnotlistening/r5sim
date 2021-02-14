/*
 * Test code - let's see if we can make a simple test to keep our
 * r5 simple_core mostly functional.
 */

#include "conftest.h"
#include "symbols.h"
#include "csr.h"

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

static void ct_run_test_submodule(ct_test_list_fn fn,
				  u32 *pass, u32 *fail)
{
	const struct ct_test *tests;
	int ret;

	*pass = 0;
	*fail = 0;

	tests = fn();

	while (tests->fn != NULL) {
		printf("Executing %-30s ", tests->name);
		ret = tests->fn(tests->data);

		printf("%s\n", ret ? "Pass!" : "Fail!");

		if (ret == 0)
			*fail += 1;
		else
			*pass += 1;

		tests++;
	}
}

/*
 * Compute a - b and store the result in dst.
 */
void ct_time_diff(struct ct_time *dst,
		  struct ct_time *a,
		  struct ct_time *b)
{
	dst->lo = a->lo - b->lo;
	dst->hi = a->hi - b->hi;

	if (a->lo < b->lo) {
		dst->hi -= 1;
		dst->lo = 0xffffffff - b->lo + a->lo + 1;
	}
}

void ct_ptime(struct ct_time *t, const char *str)
{
	printf("%s {%u.%u}\n", str, t->hi, t->lo);
}

void start(void)
{
	u32 pass_total = 0, pass;
	u32 fail_total = 0, fail;
	ct_test_list_fn *submodule = submodules;
	struct ct_time start, end, diff;
	u32 cycles_start = 0, cycles_end = 0;

	ct_rdtime(&start);
	cycles_start = ct_rdcycle();

	printf("Welcome to a super simple R5 conf test\n");

	ct_sys_info();
	trap_setup();

	while (*submodule != NULL) {
		ct_run_test_submodule(*submodule, &pass, &fail);

		pass_total += pass;
		fail_total += fail;

		submodule++;
	}

	cycles_end = ct_rdcycle();
	ct_rdtime(&end);
	ct_time_diff(&diff, &end, &start);

	printf("\n\nTest results:\n");
	printf("Passing tests: %u\n", pass_total);
	printf("Failing tests: %u\n", fail_total);
	printf("Total tests:   %u\n", pass_total + fail_total);
	printf("\n\n");

	printf("Backtrace test:\n");
	backtrace_test();
	printf("\n\n");

	printf("Cycles:          %u\n", cycles_end - cycles_start);
	printf("Time taken (ns): %u:%u\n", diff.hi, diff.lo);

	/*
	 * Quick and dirty calculation for how many MIPS we are executing.
	 * This assumes that diff.hi is 0 - e.g this takes less than 4 ish
	 * seconds to execute (what type of programmer are you?! A lazy
	 * one). Accuracy is in the ms range; that is we round our ns
	 * counter to ms so that when we divide by time, we don't just wind
	 * up with 0.
	 */
	printf("IPS:             %u\n",
	       ((cycles_end - cycles_start) /
		(diff.lo / 1000000)) * 1000);

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
	u32 pass_total = 0, pass;
	u32 fail_total = 0, fail;
	ct_test_list_fn *submodule = submodules_sv;

	printf("-- Executing Supervisor mode tests! --\n");

	while (*submodule != NULL) {
		ct_run_test_submodule(*submodule, &pass, &fail);

		pass_total += pass;
		fail_total += fail;

		submodule++;
	}

	printf("\n\nTest results:\n");
	printf("Passing tests: %u\n", pass_total);
	printf("Failing tests: %u\n", fail_total);
	printf("Total tests:   %u\n", pass_total + fail_total);
	printf("\n\n");

	while (1);
}
