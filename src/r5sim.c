/*
 * Main entrace for the r5sim program.
 */

#include <stdio.h>
#include <getopt.h>

#include <r5sim/log.h>
#include <r5sim/app.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

static struct r5sim_app_args app_args = { };

static struct option app_opts[] = {
	{ "help",		0, NULL, 'h' },
	{ "verbose",		0, NULL, 'v' },
	{ "quiet",		0, NULL, 'q' },
	{ "bootrom",		1, NULL, 'b' },
	{ "disk",		1, NULL, 'd' },
	{ "itrace",		1, NULL, 'T' },
	{ "script",		1, NULL, 's' },

	{ NULL,			0, NULL,  0  }
};

static const char *app_opts_str = "hvb:d:Ts:";

static void r5sim_help(void) {

	fprintf(stderr,
"R5 Simulator help. General usage:\n"
"\n"
"  $ r5sim [-hvqT] <-b BOOTROM> [-d <DISK>] [-s <SCRIPT>]\n"
"\n"
"Options:\n"
"\n"
"  -h,--help             Display this help message and exit.\n"
"  -v,--verbose          Increment the verbosity. Can be specified multiple\n"
"                        times.\n"
"  -q,--quiet            Decrease the verbosity. Can be specified multiple\n"
"                        times.\n"
"  -b,--bootrom          Specify a bootrom to load/execute.\n"
"  -d,--disk             Specify a file to treat as a disk. This will be loaded\n"
"                        as a VDISK device.\n"
"  -T,--itrace           Turn on instruction tracing; this is _very_ verbose.\n"
"  -s,--script           Execute a script before jumping to the BROM.\n"
"\n"
"Execute the R5 simulator; BOOTROM is a binary blob of instructions/data\n"
"that should be loaded into memory and executed. This will be the first\n"
"piece of code executed and will be placed at the base of DRAM.\n"
);

}

static void r5sim_set_default_opts(void)
{
	app_args.verbose = INFO;
}

static int r5sim_getopts(int argc, char * const argv[])
{
	int c, opt_index;

	r5sim_set_default_opts();

	while (1) {
		c = getopt_long(argc, argv,
				app_opts_str, app_opts, &opt_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			app_args.help = 1;
			break;
		case 'v':
			app_args.verbose += 1;
			break;
		case 'q':
			app_args.verbose -= 1;
			break;
		case 'b':
			app_args.bootrom = optarg;
			break;
		case 'd':
			app_args.disk_file = optarg;
			break;
		case 'T':
			app_args.itrace = 1;
			break;
		case 's':
			app_args.script = optarg;
			break;
		case '?':
			app_args.help = 1;
			return -1;
		default:
			r5sim_err("Arg bug?!\n");
			return -1;
		}
	}

	return 0;
}

struct r5sim_app_args *r5sim_app_get_args(void)
{
	return &app_args;
}

int main(int argc, char * const argv[])
{
	struct r5sim_machine *mach;
	struct r5sim_app_args *args;
	int error;

	r5sim_info("Starting RISC-V simulator.\n");

	error = r5sim_getopts(argc, argv);
	if (error) {
		r5sim_help();
		return 1;
	}

	args = r5sim_app_get_args();

	if (args->help) {
		r5sim_help();
		return 0;
	}

	mach = r5sim_machine_load_default();
	r5sim_machine_print(mach);

	r5sim_debug_init(mach);

	if (args->script) {
		char *script_args[] = {
			"exec",
			(char *)args->script,
		};

		comm_exec(mach, 2, script_args);
	}

	r5sim_machine_load_brom(mach);

	r5sim_machine_run(mach);

	return 0;
}
