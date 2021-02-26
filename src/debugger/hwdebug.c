/*
 * Provide a CLI interface for HW debugging. This gives users a sort of JTAG-
 * esque environment to control and debug a machine.
 */

#include <time.h>
#include <signal.h>

#include <r5sim/log.h>
#include <r5sim/util.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

/*
 * Argg, not much else I could think of since the signal APIs don't provide a
 * means to pass data to the signal handler.
 *
 * Store a pointer to the machine - by definition there's always one, so I think
 * this will be OK.
 */
static struct r5sim_machine *saved_mach;

static void signal_debug(int sig)
{
	/*
	 * All we do here is set debug in the saved machine: this tells
	 * the exec loop to break.
	 */
	saved_mach->debug = 1;
}

void r5sim_debug_init(struct r5sim_machine *mach)
{
	struct sigaction saction = {
		.sa_handler = signal_debug,
	};

	mach->debug = 0;
	saved_mach = mach;

	/*
	 * Prep our debug signal.
	 */
	sigemptyset(&saction.sa_mask);
	sigaction(SIGTSTP, &saction, NULL);
}
