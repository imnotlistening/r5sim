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
 * Execute a list of passed tests.
 */

#include <ct/tests.h>
#include <ct/conftest.h>

static void ct_run_test_submodule(ct_test_list_fn fn,
				  u32 *pass, u32 *fail)
{
	const struct ct_test *tests;
	int ret;

	*pass = 0;
	*fail = 0;

	tests = fn();

	while (tests->fn != NULL) {
		printf("Executing %-30s ", tests->name);
		ret = tests->fn(tests->data);

		printf("%s\n", ret ? "Pass!" : "Fail!");

		if (ret == 0)
			*fail += 1;
		else
			*pass += 1;

		tests++;
	}
}

void ct_exec_tests(ct_test_list_fn *submodule)
{
	u32 pass_total = 0, pass;
	u32 fail_total = 0, fail;


	while (*submodule != NULL) {
		ct_run_test_submodule(*submodule, &pass, &fail);

		pass_total += pass;
		fail_total += fail;

		submodule++;
	}

	printf("\n\nTest results:\n");
	printf("Passing tests: %u\n", pass_total);
	printf("Failing tests: %u\n", fail_total);
	printf("Total tests:   %u\n", pass_total + fail_total);
	printf("\n\n");

}
