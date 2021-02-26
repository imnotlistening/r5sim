/*
 * Provide an interface to read/write CSRs via the debugger.
 *
 * Usage
 *
 *   $ csr [-s <value>] <CSR> [CSR ...]
 *
 * By default print the value of CSR specified either as a number or
 * symbolically (e.g MSTATUS). If `-s' is specified then set the
 * value of the CSR to the specified value. This is treated as an
 * M-Mode write.
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <r5sim/csr.h>
#include <r5sim/core.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

static struct option csr_opts[] = {
	{ "help",		0, NULL, 'h' },
	{ "write",		1, NULL, 'w' },
	{ "set",		1, NULL, 's' },
	{ "clear",		1, NULL, 'c' },
	{ "print-all",		1, NULL, 'a' },

	{ NULL,			0, NULL,  0  }
};

static const char *csr_opts_str = "hw:s:c:a";

struct comm_args {
	u32 print_all;

	u32 value_w;
	u32 value_s;
	u32 value_c;

	u32 do_value_w;
	u32 do_value_s;
	u32 do_value_c;
};

static const char *csr_to_str(struct r5sim_machine *mach, u32 csr)
{
	if (csr > 4096)
		return NULL;

	return mach->core->csr_file[csr].name;
}

void print_all_csrs(struct r5sim_machine *mach)
{
	u32 i;

	for (i = 0; i < 4096; i++) {
		u32 csr_val;
		const char *name;
		struct r5sim_csr *csr_reg;

		csr_reg = __csr_always(mach->core, 0, i);
		if (!csr_reg)
			continue;

		csr_val = __raw_csr_read(csr_reg);
		name = csr_reg->name + 4; /*  Remove CSR_ */

		printf("%-12s 0x%08x\n", name, csr_val);
	}
}

static u32 parse_csr(struct r5sim_machine *mach, const char *str)
{
	u32 i, csr;
	char *ok;
	struct r5sim_csr *csrs = mach->core->csr_file;

	csr = (u32)strtol(str, &ok, 0);
	if (*ok == 0)
		return csr;

	/*
	 * If it wasn't a number try looking up the CSR by name.
	 */
	for (i = 0; i < 4096; i++) {
		struct r5sim_csr *c = &csrs[i];
		const char *name;

		if (!(c->flags & CSR_F_PRESENT))
			continue;

		/*
		 * CSRs are all named CSR_... thus to get the name
		 * without the CSR_ prefix add 4 to the name.
		 */
		name = c->name + 4;

		if (strncmp(name, str, strlen(name)) == 0)
			return i;
	}

	return -1;
}

static int parse_csr_opts(struct r5sim_machine *mach,
			  struct comm_args *args,
			  int argc, char *argv[])
{
	int c, opt_index;
	char *ok;

	while (1) {
		c = getopt_long(argc, argv,
				csr_opts_str, csr_opts, &opt_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			/* TODO... */
			break;
		case 'w':
			args->value_w = (u32)strtol(optarg, &ok, 0);
			if (*ok != 0) {
				printf("Failed to parse number: '%s'\n",
				       optarg);
				return -1;
			}
			args->do_value_w = 1;
			break;
		case 's':
			args->value_s = (u32)strtol(optarg, &ok, 0);
			if (*ok != 0) {
				printf("Failed to parse number: '%s'\n",
				       optarg);
				return -1;
			}
			args->do_value_s = 1;
			break;
		case 'c':
			args->value_c = (u32)strtol(optarg, &ok, 0);
			if (*ok != 0) {
				printf("Failed to parse number: '%s'\n",
				       optarg);
				return -1;
			}
			args->do_value_c = 1;
			break;
		case 'a':
			args->print_all = 1;
			break;
		}
	}

	if (args->print_all)
		return argc;

	if (argc - optind < 1)
		return -1;

	return optind;
}

int comm_csr(struct r5sim_machine *mach, int argc, char *argv[])
{
	int csr_ind;
	struct comm_args args = { 0 };

	csr_ind = parse_csr_opts(mach, &args, argc, argv);
	if (csr_ind < 0) {
		printf("Error parsing args!\n");
		return -1;
	}

	for ( ; csr_ind < argc; csr_ind++) {
		struct r5sim_csr *csr_reg;
		const char *name;
		u32 csr_val;
		u32 csr = parse_csr(mach, argv[csr_ind]);

		name = csr_to_str(mach, csr);
		if (!name) {
			printf("%-12s INVALID\n", argv[csr_ind]);
			continue;
		}

		if (args.do_value_w)
			__csr_w(mach->core, 0, args.value_w, csr);
		if (args.do_value_s)
			__csr_s(mach->core, 0, args.value_s, csr);
		if (args.do_value_c)
			__csr_c(mach->core, 0, args.value_c, csr);

		csr_reg = __csr_always(mach->core, 0, csr);
		csr_val = __raw_csr_read(csr_reg);

		name += 4; /* Remove CSR_ */
		printf("%-12s 0x%08x\n", name, csr_val);
	}

	print_all_csrs(mach);

	return 0;
}
