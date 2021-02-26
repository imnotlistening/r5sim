/*
 * breakpoint management!
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <r5sim/env.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

struct bp_args {
	int clear;
	int list;
	u32 address;
};

static struct option bp_opts[] = {
	{ "help",		0, NULL, 'h' },
	{ "clear",		0, NULL, 'c' },
	{ "list",		0, NULL, 'l' },
};

static const char *bp_opts_str = "hcl";

static void help(void)
{
	printf(
"Usage:\n"
"\n"
"  $ break [-hl][-c [bp]] [address]\n"
"\n"
"The following options are valid:\n"
"\n"
"  -h,--help            Print this help.\n"
"  -l,--list            Print all set breakpoints\n"
"  -c,--clear           Clear all breakpoints. If an address\n"
"                       address is supplied, only clear the\n"
"                       breakpoint at the specified address\n"
"                       (assuming one exists).\n"
"\n"
"By default, if no options are specified, set a breakpoint at the\n"
"supplied address.\n"
"\n"
		);
}

static u32 *get_free_bp(struct r5sim_machine *mach)
{
	u32 i;

	/*
	 * Find the first available (equal to 0) hw break point.
	 */
	for (i = 0; i < BREAKPOINT_NR; i++) {
		if (!mach->hwbreaks[i])
			return &mach->hwbreaks[i];
	}

	return NULL;
}

static int set_bp(struct r5sim_machine *mach, u32 address)
{
	u32 *bp = get_free_bp(mach);

	if (!bp) {
		printf("No more breakpoints!\n");
		return -1;
	}

	*bp = address;
	mach->breaks_set++;

	r5sim_assert(mach->breaks_set <= BREAKPOINT_NR);

	return 0;
}

static void clear_bps(struct r5sim_machine *mach, u32 address)
{
	u32 *bp, i;

	for (i = 0; i < BREAKPOINT_NR; i++) {
		bp = &mach->hwbreaks[i];

		if (*bp == 0)
			continue;

		if (address && *bp != address)
			continue;

		*bp = 0;
		mach->breaks_set--;
	}

	r5sim_assert(mach->breaks_set >= 0);
}

static void list_bps(struct r5sim_machine *mach)
{
	u32 *bp, i;

	printf("Currently set breakpoints:\n");
	for (i = 0; i < BREAKPOINT_NR; i++) {
		bp = &mach->hwbreaks[i];

		if (*bp == 0)
			continue;

		printf("  BP @ 0x%08x\n", *bp);
	}
}

static int parse_args(struct bp_args *args, int argc, char *argv[])
{
	int c, opt_index;
	char *ok;

	while (1) {
		c = getopt_long(argc, argv,
				bp_opts_str, bp_opts, &opt_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			help();
			return -1;
		case 'l':
			args->list = 1;
			break;
		case 'c':
			args->clear = 1;
			break;
		case '?':
		default:
			return -1;
		}
	}

	if (argc - optind > 1) {
		help();
		return -1;
	}

	if (argc - optind == 1) {
		args->address = (u32)strtol(argv[optind], &ok, 0);
		if (*ok != 0) {
			printf("Failed to parse number: '%s'\n",
			       optarg);
			return -1;
		}
	}

	return 0;
}

/*
 * Manage break points.
 */
int comm_break(struct r5sim_machine *mach, int argc, char *argv[])
{
	struct bp_args args = { };

	/*
	 * Parse arguments.
	 */
	if (parse_args(&args, argc, argv))
		return -1;

	/*
	 * Handle a potential clear: could be either all or based on
	 * a specific address.
	 */
	if (args.clear) {
		clear_bps(mach, args.address);
		return 0;
	}

	if (args.list) {
		list_bps(mach);
		return 0;
	}

	/*
	 * Finally, if we are here, set a breakpoint.
	 */
	if (!args.address) {
		printf("Missing address to set BP at!\n");
		return -1;
	}

	return set_bp(mach, args.address);
}
