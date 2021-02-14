/*
 * Implement a small conformance test to see if our R5 simple core is
 * mostly functional.
 */

#ifndef __CONFTEST_H__
#define __CONFTEST_H__

#include "types.h"

/*
 * Each test sub-module should provide a list of tests to execute.
 * The test code will run through them and make sure they pass.
 * Make sure the list is NULL terminated.
 */

typedef int (*ct_func_t)(void *data);
typedef const struct ct_test *(*ct_test_list_fn)(void);

struct ct_test {
	ct_func_t	 fn;
	void		*data;
	const char	*name;
};

#define CT_TEST(__fn, __data, __name)		\
	{					\
		.fn = __fn,			\
		.data = __data,			\
		.name = __name,			\
	}

struct ct_time {
	u32 lo;
	u32 hi;
};

/*
 * This is the S-Mode test's "ABI" to ask M-Mode to delegate traps
 * to S-Mode. S-Mode will do an ECall with this variable set to true,
 * and when that happens, M-Mode will delegate all exceptions to
 * S-Mode.
 */
extern volatile u32 delegate_smode;
extern volatile u32 m_intr_exec;
extern volatile u32 s_intr_exec;
extern volatile u32 m_excep_exec;
extern volatile u32 s_excep_exec;
extern volatile u32 expect_exception;

/*
 * No need for CPU barriers on the simple_core.
 */
#define readl(addr)		*((volatile u32 *)(addr))
#define writel(addr, val)	*((volatile u32 *)(addr)) = (u32)(val)

/* Compiler barrier. */
#define barrier()		asm volatile("": : :"memory")

/*
 * Print some basic sys info.
 */
void ct_sys_info(void);

__attribute__((format (printf, 1, 2)))
int printf(const char *fmt, ...);

void start(void);
void sv_start(void);
void _smode_start(void);

int  trap_setup(void);
void trap_entrance(u32 trap_pc, u32 cause);
void __trap_vector(void);
void __trap_vector_sv(void);

void trap_entrance_sv(u32 trap_pc, u32 cause);

void backtrace(void);
int  backtrace_test(void);

void ct_time_diff(struct ct_time *dst, struct ct_time *a, struct ct_time *b);
void ct_ptime(struct ct_time *t, const char *str);

/*
 * Defined in entry.S
 */
u32  ct_rdcycle(void);
u32  ct_rdinstret(void);
void ct_rdtime(struct ct_time *time);

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


#define TRAP_ILLEGAL_INST	2
#define TRAP_ECALL_MMODE	11

#endif
