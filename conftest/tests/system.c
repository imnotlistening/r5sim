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

#include <ct/time.h>
#include <ct/tests.h>
#include <ct/conftest.h>

static int
ct_test_rdcycle(void *data)
{
	u32 cycles, cycles_end;
	u32 a = 4, b = 6;

	cycles = ct_rdcycle();
	barrier();
	a += b;
	barrier();
	cycles_end = ct_rdcycle();

	return cycles_end > cycles;
}

static int
ct_test_rdinstret(void *data)
{
	u32 instret, instret_end;
	u32 a = 4, b = 6;

	instret = ct_rdinstret();
	barrier();
	a += b;
	barrier();
	instret_end = ct_rdinstret();

	return instret_end > instret;
}

static int
ct_test_rdtime(void *data)
{
	struct ct_time start, end, diff;
	int i;

	ct_rdtime(&start);

	for (i = 0; i < 10000; i++)
		;

	ct_rdtime(&end);

	ct_time_diff(&diff, &end, &start);

	return diff.lo != 0;
}

static const struct ct_test system_tests[] = {
	CT_TEST(ct_test_rdcycle,		NULL,			"system_rdcycle"),
	CT_TEST(ct_test_rdinstret,		NULL,			"system_rdinstret"),
	CT_TEST(ct_test_rdtime,			NULL,			"system_rdtime"),

	/*
	 * NULL terminate.
	 */
	CT_TEST(NULL,				NULL,			NULL),
};

const struct ct_test *
ct_system(void)
{
	return system_tests;
}
