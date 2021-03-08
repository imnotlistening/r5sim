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
 * Test for load and store instructions.
 */

#include <ct/conftest.h>
#include <ct/tests.h>

struct ct_chars_list {
	char		a;
	char		b;
	char		c;
	char		d;
	u32		sum;
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
	u32 sum;

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
