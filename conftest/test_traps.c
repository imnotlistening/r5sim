/*
 * Simple tests to make sure that what traps we can test without
 * an MMU are mostly working.
 *
 * This basically just issues a bad instruction and then expects
 * the trap to skip said instruction.
 */

#include "conftest.h"

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
	asm volatile(".long 0x00000000\n\t");

	return 1;
}
static int ct_test_ecall(void *data)
{
	asm volatile("ecall\n\t");

	return 1;
}

static const struct ct_test op_traps[] = {
	CT_TEST(ct_test_access_align,		NULL,			"access_align"),
	CT_TEST(ct_test_illegal_inst,		NULL,			"illegal_inst"),
	CT_TEST(ct_test_ecall,			NULL,			"ecall"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *ct_traps(void)
{
	return op_traps;
}
