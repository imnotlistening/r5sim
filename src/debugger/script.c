/*
 * Run "scripts" of a sort; these scripts can contain debugger commands
 * like CSRs to prime an environment for debugging purposes.
 */

#include <stdio.h>
#include <string.h>

#include <r5sim/hwdebug.h>

/* Should be enough for anyone. */
#define LINE_BUF_SZ		768

static void usage(void)
{
	printf(
"Execute a script. Usage\n"
"\n"
"  $ exec <script>\n"
"\n"
		);
}

static int open_and_exec(struct r5sim_machine *mach, const char *script)
{
	FILE *script_file;
	char *line;
	char line_buf[LINE_BUF_SZ], saved_line[LINE_BUF_SZ];
	int line_nr = 0;
	int ret = 0;

	script_file = fopen(script, "r");
	if (!script_file) {
		perror(script);
		return -1;
	}

	while ((line = fgets(line_buf, LINE_BUF_SZ, script_file)) != NULL) {
		line_nr++;

		strncpy(saved_line, line_buf, LINE_BUF_SZ);
		if (saved_line[strlen(saved_line) - 1] == '\n')
			saved_line[strlen(saved_line) - 1] = 0;

		ret = r5sim_debug_exec_line(mach, line);
		if (ret < 0) {
			printf("%s: Error @ line %d\n", script, line_nr);

			ret = -1;
			goto done;
		}
	}

done:
	fclose(script_file);
	return ret;
}

/*
 * Execute the specified script of debugger commands. Usage:
 *
 *   $ exec <script>
 *
 * This rejects further arguments (for now) since in the future it
 * may be possible to pass arguments to a script.
 */
int comm_exec(struct r5sim_machine *mach, int argc, char *argv[])
{
	if (argc != 2) {
		usage();
		return -1;
	}

	return open_and_exec(mach, argv[1]);
}
