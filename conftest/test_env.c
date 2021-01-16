/*
 * Simple tests to make sure our environment is sane.
 */

#include "conftest.h"

static int
ct_test_uint8_t(void *data)
{
	return sizeof(uint8_t) == 1;
}

static int
ct_test_uint16_t(void *data)
{
	return sizeof(uint16_t) == 2;
}


static int
ct_test_uint32_t(void *data)
{
	return sizeof(uint32_t) == 4;
}

static const struct ct_test env_tests[] = {
	CT_TEST(ct_test_uint8_t,		NULL,			"sizeof_uint8_t"),
	CT_TEST(ct_test_uint16_t,		NULL,			"sizeof_uint16_t"),
	CT_TEST(ct_test_uint32_t,		NULL,			"sizeof_uint32_t"),

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
