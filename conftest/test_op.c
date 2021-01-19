/*
 * Simple tests to make sure that the 3 register ops work.
 *
 * At some level, if these ops didn't work, then this test probably
 * just wouldn't run, period. But it's good for completeness.
 */

#include "conftest.h"

test_op(add1, 2,		4,		6);
test_op(add2, -2,		-4,		-6);
test_op(add3, 0xffffffff,	4,		3);
test_op(add4, -1,		-1,		-2);
test_op(add5, 0,		0,		0);
test_op(add6, -2,		4,		2);

test_op(sub1, 2,		4,		-2);
test_op(sub2, -2,		-4,		2);
test_op(sub3, 0,		1,		-1);
test_op(sub4, -1,		-1,		0);
test_op(sub5, 0,		-10,		10);
test_op(sub6, -2,		4,		-6);

test_op(sll1, 2,		1,		4);
test_op(sll2, 0,		5,		0);
test_op(sll3, 0xffffffff,	31,		0x80000000);
test_op(sll4, 0xffffffff,	32,		0xffffffff);

test_op(slt1, 2,		1,		0);
test_op(slt2, 1,		2,		1);
test_op(slt3, -1,		1,		1);
test_op(slt4, 1,		-1,		0);
test_op(slt5, -4,		-1,		1);

test_op(sltu1, 2,		1,		0);
test_op(sltu2, 1,		2,		1);
test_op(sltu3, 0xffffffff,	2,		0);

test_func(add)
test_func(sub)
test_func(sll)
test_func(slt)
test_func(sltu)

static const struct ct_test op_tests[] = {
	CT_TEST(ct_test_add,			&add1,			"add1"),
	CT_TEST(ct_test_add,			&add2,			"add2"),
	CT_TEST(ct_test_add,			&add3,			"add3"),
	CT_TEST(ct_test_add,			&add4,			"add4"),
	CT_TEST(ct_test_add,			&add5,			"add5"),
	CT_TEST(ct_test_add,			&add6,			"add6"),

	CT_TEST(ct_test_sub,			&sub1,			"sub1"),
	CT_TEST(ct_test_sub,			&sub2,			"sub2"),
	CT_TEST(ct_test_sub,			&sub3,			"sub3"),
	CT_TEST(ct_test_sub,			&sub4,			"sub4"),
	CT_TEST(ct_test_sub,			&sub5,			"sub5"),
	CT_TEST(ct_test_sub,			&sub6,			"sub6"),

	CT_TEST(ct_test_sll,			&sll1,			"sll1"),
	CT_TEST(ct_test_sll,			&sll2,			"sll2"),
	CT_TEST(ct_test_sll,			&sll3,			"sll3"),
	CT_TEST(ct_test_sll,			&sll4,			"sll4"),

	CT_TEST(ct_test_slt,			&slt1,			"slt1"),
	CT_TEST(ct_test_slt,			&slt2,			"slt2"),
	CT_TEST(ct_test_slt,			&slt3,			"slt3"),
	CT_TEST(ct_test_slt,			&slt4,			"slt4"),
	CT_TEST(ct_test_slt,			&slt5,			"slt5"),

	CT_TEST(ct_test_sltu,			&sltu1,			"sltu1"),
	CT_TEST(ct_test_sltu,			&sltu2,			"sltu2"),
	CT_TEST(ct_test_sltu,			&sltu3,			"sltu3"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_op(void)
{
	return op_tests;
}
