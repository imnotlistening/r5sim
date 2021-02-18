/*
 * HW debugging facilities. This provides a CLI that gives a JTAG sort of
 * experience. This provides access to registers, memory, etc.
 */

#ifndef __R5SIM_HWDEBUG_H__
#define __R5SIM_HWDEBUG_H__

struct r5sim_machine;

typedef int (*r5sim_comm_func)(struct r5sim_machine *mach,
			       int argc, const char **argv);

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


/*
 * Commands should retern COMM_OK or 0 for success and less than 0 for an
 * error. For a command that breaks out of the debug session and back
 * into regular run mode, return COMM_RUN.
 */
#define COMM_OK		0
#define COMM_RUN	1

void r5sim_debug_init(struct r5sim_machine *mach);
void r5sim_debug_do_session(struct r5sim_machine *mach);

#endif
