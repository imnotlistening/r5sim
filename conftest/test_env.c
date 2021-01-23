/*
 * Simple tests to make sure our environment is sane.
 */

#include "conftest.h"

static int
ct_test_u8(void *data)
{
	return sizeof(u8) == 1;
}

static int
ct_test_u16(void *data)
{
	return sizeof(u16) == 2;
}


static int
ct_test_u32(void *data)
{
	return sizeof(u32) == 4;
}

static const struct ct_test env_tests[] = {
	CT_TEST(ct_test_u8,		NULL,			"sizeof_u8"),
	CT_TEST(ct_test_u16,		NULL,			"sizeof_u16"),
	CT_TEST(ct_test_u32,		NULL,			"sizeof_u32"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_env(void)
{
	return env_tests;
}
