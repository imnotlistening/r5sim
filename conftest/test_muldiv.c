/*
 * Simple tests to make sure that multiply and divide work.
 */

#include "conftest.h"

struct ct_op {
	uint32_t a;
	uint32_t b;
	uint32_t answer;
};

#define test_op(name, __a, __b, __answer)	\
	struct ct_op name = {			\
		.a = __a,			\
		.b = __b,			\
		.answer = __answer,		\
	}

#define test_func(op)					\
	static int					\
	ct_test_##op(void *data)			\
	{						\
		struct ct_op *test = data;		\
		uint32_t res;				\
							\
		asm(#op "	%0, %1, %2\n\t"		\
		    : "=r" (res)			\
		    : "r" (test->a), "r" (test->b));	\
							\
		return res == test->answer;		\
	}

test_op(mul1, 2,		4,		8);
test_op(mul2, -2,		-4,		8);
test_op(mul3, -2,		4,		-8);
test_op(mul4, 1024,		4*1024*1024,	0);
test_op(mul5, 1025,		4*1024*1024,	4194304);
test_op(mul6, -1025,		4*1024*1024,	-4194304);

test_op(mulh1, 2,		4,		0);
test_op(mulh2, -2,		-4,		0);
test_op(mulh3, -2,		4,		0xffffffff);
test_op(mulh4, 1024,		4*1024*1024,	1);
test_op(mulh5, 1025,		4*1024*1024,	1);
test_op(mulh6, 0xffffffff,	0xffffffff,	0);

test_op(mulhsu1, 2,		4,		0);
test_op(mulhsu2, -2,		4,		0xffffffff);
test_op(mulhsu3, 1024,		4*1024*1024,	1);
test_op(mulhsu4, 1025,		4*1024*1024,	1);
test_op(mulhsu5, 0xffffffff,	0xffffffff,	0xffffffff);

test_op(mulhu1, 1,		0xffffffff,	0);
test_op(mulhu2, 2,		0xffffffff,	1);
test_op(mulhu3, 1024,		0xffffffff,	1023);
test_op(mulhu4, 1025,		0xffffffff,	1024);
test_op(mulhu5, 0xffffffff,	0xffffffff,	0xfffffffe);

test_op(div1,	16,		2,		8);
test_op(div2,	-16,		2,		-8);
test_op(div3,	1024,		-256,		-4);
test_op(div4,	1025,		256,		4);
test_op(div5,	0xffffffff,	0xffffffff,	1);

test_op(divu1,	16,		2,		8);
test_op(divu2,	16,		1,		16);
test_op(divu3,	1024,		100,		10);

test_op(rem1,	16,		2,		0);
test_op(rem2,	-16,		3,		-1);
test_op(rem3,	1024,		-100,		24);
test_op(rem4,	1,		1,		0);
test_op(rem5,	0xffffffff,	0xffffffff,	0);

test_op(remu1,	16,		2,		0);
test_op(remu2,	100,		30,		10);
test_op(remu3,	12,		5,		2);

/*
 * Test functions for each multiply op.
 */
test_func(mul)
test_func(mulh)
test_func(mulhsu)
test_func(mulhu)
test_func(div)
test_func(divu)
test_func(rem)
test_func(remu)

static const struct ct_test muldiv_tests[] = {
	CT_TEST(ct_test_mul,			&mul1,			"mul1"),
	CT_TEST(ct_test_mul,			&mul2,			"mul2"),
	CT_TEST(ct_test_mul,			&mul3,			"mul3"),
	CT_TEST(ct_test_mul,			&mul4,			"mul4"),
	CT_TEST(ct_test_mul,			&mul5,			"mul5"),
	CT_TEST(ct_test_mul,			&mul6,			"mul6"),

	CT_TEST(ct_test_mulh,			&mulh1,			"mulh1"),
	CT_TEST(ct_test_mulh,			&mulh2,			"mulh2"),
	CT_TEST(ct_test_mulh,			&mulh3,			"mulh3"),
	CT_TEST(ct_test_mulh,			&mulh4,			"mulh4"),
	CT_TEST(ct_test_mulh,			&mulh5,			"mulh5"),
	CT_TEST(ct_test_mulh,			&mulh6,			"mulh6"),

	CT_TEST(ct_test_mulhsu,			&mulhsu1,		"mulhsu1"),
	CT_TEST(ct_test_mulhsu,			&mulhsu2,		"mulhsu2"),
	CT_TEST(ct_test_mulhsu,			&mulhsu3,		"mulhsu3"),
	CT_TEST(ct_test_mulhsu,			&mulhsu4,		"mulhsu4"),
	CT_TEST(ct_test_mulhsu,			&mulhsu5,		"mulhsu5"),

	CT_TEST(ct_test_mulhu,			&mulhu1,		"mulhu1"),
	CT_TEST(ct_test_mulhu,			&mulhu2,		"mulhu2"),
	CT_TEST(ct_test_mulhu,			&mulhu3,		"mulhu3"),
	CT_TEST(ct_test_mulhu,			&mulhu4,		"mulhu4"),
	CT_TEST(ct_test_mulhu,			&mulhu5,		"mulhu5"),

	CT_TEST(ct_test_div,			&div1,			"div1"),
	CT_TEST(ct_test_div,			&div2,			"div2"),
	CT_TEST(ct_test_div,			&div3,			"div3"),
	CT_TEST(ct_test_div,			&div4,			"div4"),
	CT_TEST(ct_test_div,			&div5,			"div5"),

	CT_TEST(ct_test_divu,			&divu1,			"divu1"),
	CT_TEST(ct_test_divu,			&divu2,			"divu2"),
	CT_TEST(ct_test_divu,			&divu3,			"divu3"),

	CT_TEST(ct_test_rem,			&rem1,			"rem1"),
	CT_TEST(ct_test_rem,			&rem2,			"rem2"),
	CT_TEST(ct_test_rem,			&rem3,			"rem3"),
	CT_TEST(ct_test_rem,			&rem4,			"rem4"),
	CT_TEST(ct_test_rem,			&rem5,			"rem5"),

	CT_TEST(ct_test_remu,			&remu1,			"remu1"),
	CT_TEST(ct_test_remu,			&remu2,			"remu2"),
	CT_TEST(ct_test_remu,			&remu3,			"remu3"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_muldiv(void)
{
	return muldiv_tests;
}
