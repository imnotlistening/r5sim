/* Copyright 2021, Alex Waterman <imnotlistening@gmail.com>
 *
 * This file is part of r5sim.
 *
 * r5sim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * r5sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with r5sim.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Simple tests to make sure our environment is sane.
 */

#include <ct/conftest.h>
#include <ct/tests.h>

static int ct_test_u8(void *data)
{
	return sizeof(u8) == 1;
}

static int ct_test_u16(void *data)
{
	return sizeof(u16) == 2;
}

static int ct_test_u32(void *data)
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

const struct ct_test *ct_env(void)
{
	return env_tests;
}
