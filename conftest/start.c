/*
 * Test code - let's see if we can make a simple test to keep our
 * r5 simple_core mostly functional.
 */

#include "conftest.h"

static ct_test_list_fn submodules[] = {
	ct_env,
	ct_load_store,
	NULL
};

static void
ct_run_test_submodule(ct_test_list_fn fn, uint32_t *pass, uint32_t *fail)
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

void
start(void)
{
	uint32_t pass_total = 0, pass;
	uint32_t fail_total = 0, fail;
	ct_test_list_fn *submodule = submodules;

	printf("Welcome to a super simple R5 conf test\n");

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

	while (1);
}
