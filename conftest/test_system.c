/*
 * Simple tests to make sure our environment is sane.
 */

#include "conftest.h"

static int
ct_test_rdcycle(void *data)
{
	uint32_t cycles, cycles_end;
	uint32_t a = 4, b = 6;

	cycles = ct_rdcycle();
	barrier();
	a += b;
	barrier();
	cycles_end = ct_rdcycle();

	return cycles_end > cycles;
}

static int
ct_test_rdinstret(void *data)
{
	uint32_t instret, instret_end;
	uint32_t a = 4, b = 6;

	instret = ct_rdinstret();
	barrier();
	a += b;
	barrier();
	instret_end = ct_rdinstret();

	return instret_end > instret;
}

static int
ct_test_rdtime(void *data)
{
	struct ct_time start, end, diff;
	int i;

	ct_rdtime(&start);

	for (i = 0; i < 10000; i++)
		;

	ct_rdtime(&end);

	ct_time_diff(&diff, &end, &start);

	return diff.lo != 0;
}

static const struct ct_test system_tests[] = {
	CT_TEST(ct_test_rdcycle,		NULL,			"system_rdcycle"),
	CT_TEST(ct_test_rdinstret,		NULL,			"system_rdinstret"),
	CT_TEST(ct_test_rdtime,			NULL,			"system_rdtime"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_system(void)
{
	return system_tests;
}
