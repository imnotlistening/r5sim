/*
 * HW debugging session; this handles commands and output.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <r5sim/log.h>
#include <r5sim/core.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

/*
 * $ load <file> <address>
 *
 * Load a file to the passed address in DRAM. This address must be
 * in DRAM since we copy directly to DRAM and don't go through the
 * mem access functions in the machine.
 */

/*
 * $ core
 *
 * Print the state of the core.
 */
static int comm_core(struct r5sim_machine *mach,
		     int argc, const char *argv[])
{
	int i;
	struct r5sim_core *core = mach->core;

	printf("Core state:\n");
	printf("  PC:      0x%08x\n",      core->pc);
	printf("  MSTATUS: 0x%08x\n", core->mstatus);
	printf("  Priv:    %d\n",     core->priv);
	printf("  MIE:     0x%08x\n", core->mie);
	printf("  MIP:     0x%08x\n", core->mip);

	printf("Registers:\n");
	for (i = 0; i < 32; i += 4) {
		printf("  %3s: 0x%08x  %3s: 0x%08x  %3s: 0x%08x  %3s: 0x%08x\n",
		       r5sim_reg_to_str(i + 0), core->reg_file[i + 0],
		       r5sim_reg_to_str(i + 1), core->reg_file[i + 1],
		       r5sim_reg_to_str(i + 2), core->reg_file[i + 2],
		       r5sim_reg_to_str(i + 3), core->reg_file[i + 3]);
	}

	return 0;
}

/*
 * $ m <address> [length]
 *
 * Display memory at the requested address for the length bytes.
 * If length is not specified then 32 bytes are displayed. Both
 * address and length will be truncated to the nearest 4 bytes.
 */
static int comm_m(struct r5sim_machine *mach,
		  int argc, const char *argv[])
{
	char *end_ptr;
	u32 address;
	u32 length = 32;
	u32 i;

	if (argc == 1 || argc > 3) {
		printf("Usage:\n");
		printf("  %s <address> [length]\n", argv[0]);
		return -1;
	}

	address = strtol(argv[1], &end_ptr, 0);
	if (*end_ptr != 0) {
		printf("Failed to convert '%s' to u32!\n", argv[1]);
		return -1;
	}

	if (argc == 3) {
		length = strtol(argv[2], &end_ptr, 0);
		if (*end_ptr != 0) {
			printf("Failed to convert '%s' to u32!\n", argv[2]);
			return -1;
		}
	}

	/*
	 * Now, time to print the memory!
	 */
	address &= ~0x3;
	length  &= ~0x3;

	for (i = 0; i < length; i += 4) {
		u32 mem = 0xffffffff;

		if (i % 16 == 0)
			printf("0x%08x:", address + i);

		mach->memload32(mach, address + i, &mem);
		printf(" 0x%08x", mem);

		if (i % 16 == 12)
			printf("\n");
	}

	if (i % 16 != 0)
		printf("\n");

	return 0;
}

/*
 * $ run [PC-value]
 */
static int comm_run(struct r5sim_machine *mach,
		    int argc, const char *argv[])
{
	u32 pc;
	char *end_ptr;

	/*
	 * Just run with whatever the core state currently is.
	 */
	if (argc == 1)
		return COMM_RUN;

	/*
	 * If we have more than one argument, then we aren't called
	 * correctly. Do nothing.
	 */
	if (argc > 2) {
		printf("Usage:\n");
		printf("  %s [PC-value]\n", argv[0]);
		return -1;
	}

	/*
	 * If we are here, we have a PC to set and then jump to.
	 */
	pc = strtol(argv[1], &end_ptr, 0);
	if (*end_ptr != 0) {
		printf("Failed to convert '%s' to u32!\n", argv[1]);
		return -1;
	}

	mach->core->pc = pc;

	return COMM_RUN;
}

static struct r5sim_hwd_command commands[] = {
	CMD("run",  comm_run,  "Run the simulator"),
	CMD("m",    comm_m,    "Dump memory"),
	CMD("core", comm_core, "Dump core state"),

	CMD(NULL,  NULL,     NULL)
};

static struct r5sim_hwd_command *lookup_command(const char *cmd_str)
{
	struct r5sim_hwd_command *cmd = commands;

	while (cmd->name && strcmp(cmd->name, cmd_str) != 0)
		cmd++;

	return cmd->name ? cmd : NULL;
}

static int compute_max_argc(const char *line)
{
	int argc = 1;

	while (*line) {
		if (*line++ == ' ')
			argc++;
	}

	return argc;
}

/*
 * Process a line and return 0 so long as an exit has not been requested.
 * Any other error is considered not fatal and the user can just type in
 * another command.
 */
static int process_line(struct r5sim_machine *mach, char *line)
{
	struct r5sim_hwd_command *cmd;
	const char **comm_argv;
	int comm_argc;
	int i, ret = 0;

	// r5sim_info("Processing command: '%s'\n", line);

	if (!*line)
		return 0;

	comm_argc = compute_max_argc(line);
	comm_argv = malloc(sizeof(char *) * comm_argc);
	r5sim_assert(comm_argv != NULL);

	/*
	 * Build the command's input argv list.
	 */
	i = 1;
	comm_argv[0] = strtok(line, " ");

	while (1) {
		comm_argv[i] = strtok(NULL, " ");
		if (!comm_argv[i])
			break;
		i++;
	}

	/*
	 * The real argc; computed from the actual number of tokens
	 * generated.
	 */
	comm_argc = i;

	/*
	 * Look up the command struct from comm_argv[0].
	 */
	cmd = lookup_command(comm_argv[0]);
	if (!cmd) {
		printf("Unknown command: '%s'\n", comm_argv[0]);
		goto done;
	}

	/*
	 * And finally execute it.
	 */
	ret = cmd->func(mach, comm_argc, comm_argv);

done:
	free(comm_argv);
	// printf("Done line!\n");
	return ret;
}

static char *strip_whitespace(char *line)
{
	int end;

	/* Goodbye leading whitespace. */
	while (isspace(*line))
		line++;

	end = strlen(line) - 1;

	/* And goodbye trailing whitespace. */
	while (isspace(line[end]) && end >= 0)
		line[end--] = '\0';

	return line;
}

/*
 * Do a debug session!
 */
void r5sim_debug_do_session(struct r5sim_machine *mach)
{
	char *line_raw, *line;
	int ret;

	while (1) {
		line_raw = readline("dbg $ ");
		if (!line_raw) {
			printf("\n");
			continue;
		}

		line = strip_whitespace(line_raw);

		if (*line)
			add_history(line);

		ret = process_line(mach, line);

		free(line_raw);

		if (ret == COMM_RUN)
			break;
	}

	return;
}
