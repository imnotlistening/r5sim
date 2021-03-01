/*
 * HW debugging facilities. This provides a CLI that gives a JTAG sort of
 * experience. This provides access to registers, memory, etc.
 */

#ifndef __R5SIM_HWDEBUG_H__
#define __R5SIM_HWDEBUG_H__

#include <r5sim/machine.h>

typedef int (*r5sim_comm_func)(struct r5sim_machine *mach,
			       int argc, char **argv);

struct r5sim_hwd_command {
	r5sim_comm_func		 func;
	const char		*name;
	const char		*help;
};

#define CMD(__name, __func, __help)		\
	{					\
		.func = __func,			\
		.name = __name,			\
		.help = __help,			\
	}

static inline int r5sim_hwbreak(struct r5sim_machine *mach, u32 pc)
{
	u32 i;
	int do_break = 0;

	if (!mach->breaks_set)
		return 0;

	for (i = 0; i < BREAKPOINT_NR; i++)
		do_break += (mach->hwbreaks[i] == pc);

	return do_break;
}

/*
 * Commands should retern COMM_OK or 0 for success and less than 0 for an
 * error. For a command that breaks out of the debug session and back
 * into regular run mode, return COMM_RUN.
 */
#define COMM_OK		0
#define COMM_RUN	1

void r5sim_debug_init(struct r5sim_machine *mach);
void r5sim_debug_do_session(struct r5sim_machine *mach);
int  r5sim_debug_exec_line(struct r5sim_machine *mach, char *line);

/*
 * Commands to do debugging!
 */
int comm_csr(struct r5sim_machine *mach, int argc, char *argv[]);
int comm_break(struct r5sim_machine *mach, int argc, char *argv[]);
int comm_exec(struct r5sim_machine *mach, int argc, char *argv[]);

#endif
