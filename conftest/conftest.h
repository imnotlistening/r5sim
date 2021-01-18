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
	uint32_t lo;
	uint32_t hi;
};

/*
 * No need for barriers on the simple_core.
 */
#define readl(addr)		*((uint32_t *)(addr))
#define writel(addr, val)	*((uint32_t *)(addr)) = (uint32_t)(val)

/* Compiler barrier. */
#define barrier()		asm volatile("": : :"memory")

__attribute__((format (printf, 1, 2)))
int
printf(const char *fmt, ...);

void
ct_time_diff(struct ct_time *dst, struct ct_time *a, struct ct_time *b);
void
ct_ptime(struct ct_time *t, const char *str);

/*
 * Defined in conftest.S
 */
uint32_t
ct_rdcycle(void);
uint32_t
ct_rdinstret(void);
void
ct_rdtime(struct ct_time *time);

/*
 * Tests.
 */
const struct ct_test *
ct_env(void);
const struct ct_test *
ct_system(void);
const struct ct_test *
ct_load_store(void);
const struct ct_test *
ct_muldiv(void);

#endif
