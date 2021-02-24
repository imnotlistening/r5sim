/*
 * Tests and the machinary to run them.
 */

#ifndef __TESTS_H__
#define __TESTS_H__

#include <ct/types.h>

typedef int (*ct_func_t)(void *data);

struct ct_test {
	ct_func_t	 fn;
	void		*data;
	const char	*name;
};

typedef const struct ct_test *(*ct_test_list_fn)(void);

#define CT_TEST(__fn, __data, __name)		\
	{					\
		.fn = __fn,			\
		.data = __data,			\
		.name = __name,			\
	}

/*
 * This makes it easy to build lots of arithmetic ops quickly. This
 * is aimed at ops that take 3 register arguments.
 */
struct ct_op {
	u32 a;
	u32 b;
	u32 answer;
};

#define test_op(name, __a, __b, __answer)	\
	struct ct_op name = {			\
		.a = __a,			\
		.b = __b,			\
		.answer = __answer,		\
	}

#define test_func(op)					\
	static int ct_test_##op(void *data)		\
	{						\
		struct ct_op *test = data;		\
		u32 res;				\
							\
		asm(#op "	%0, %1, %2\n\t"		\
		    : "=r" (res)			\
		    : "r" (test->a), "r" (test->b));	\
							\
		return res == test->answer;		\
	}

void ct_exec_tests(ct_test_list_fn *submodules);

/*
 * Tests.
 */
const struct ct_test *ct_env(void);
const struct ct_test *ct_system(void);
const struct ct_test *ct_load_store(void);
const struct ct_test *ct_muldiv(void);
const struct ct_test *ct_op(void);
const struct ct_test *ct_traps(void);
const struct ct_test *ct_sv_traps(void);

#endif
