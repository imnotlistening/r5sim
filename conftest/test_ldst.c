/*
 * Test for load and store instructions.
 */

#include "conftest.h"

struct ct_chars_list {
	char		a;
	char		b;
	char		c;
	char		d;
	uint32_t	sum;
};

static struct ct_chars_list test1 = {
	.a = 1,
	.b = 2,
	.c = 3,
	.d = 4,
	.sum = 10,
};

/*
 * Load 4 chars, sum them, and then make sure they match the passed sum.
 */
static int
ct_test_load_bytes(void *data)
{
	struct ct_chars_list *chars = data;
	uint32_t sum;

	sum = chars->a + chars->b + chars->c + chars->d;

	return sum == chars->sum;
}

static const struct ct_test load_store_tests[] = {
	CT_TEST(ct_test_load_bytes,		&test1,			"load_bytes_test1"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_load_store(void)
{
	return load_store_tests;
}
