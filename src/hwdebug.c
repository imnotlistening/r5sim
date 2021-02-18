/*
 * Provide a CLI interface for HW debugging. This gives users a sort of JTAG-
 * esque environment to control and debug a machine.
 */

#include <time.h>
#include <signal.h>
#include <pthread.h>

#include <r5sim/log.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

/*
 * Argg, not much else I could think of since the signal APIs don't provide a
 * means to pass data to the signal handler.
 *
 * When this is set, the debug thread will detect this and begin the debug hand
 * off.
 */
static volatile int do_debug;

static void signal_debug(int sig);

static void *debug_thread(void *__mach)
{
	struct r5sim_machine *mach = __mach;
	struct timespec spec = {
		.tv_sec  = 0,
		.tv_nsec = 1000000, /* 1 millisecond */
	};
	struct sigaction saction = {
		.sa_handler = signal_debug,
	};

idle:
	while (1) {
		nanosleep(&spec, NULL);

		if (do_debug)
			break;
	}

	/*
	 * Time to actually do a debug. First let's stop the machine.
	 */
	mach->debug = 1;
	while (mach->debug)
		nanosleep(&spec, NULL);

	r5sim_debug_do_session(mach);

	/*
	 * Now that we are done with the debug session, we need to rearm the
	 * do_debug field for potential subsequent debug sessions.
	 */
	do_debug = 0;
	sigemptyset(&saction.sa_mask);
	sigaction(SIGTSTP, &saction, NULL);
	pthread_cond_signal(&mach->debug_done);
	goto idle;

	return NULL;
}

static void signal_debug(int sig)
{
	struct sigaction saction = {
		.sa_handler = SIG_IGN,
	};

	/*
	 * Not a lot is safe to do in signals; just set the global do_debug to
	 * true and mask further debug signals.
	 */
	do_debug = 1;

	sigemptyset(&saction.sa_mask);
	sigaction(SIGTSTP, &saction, NULL);
}

void r5sim_debug_init(struct r5sim_machine *mach)
{
	struct sigaction saction = {
		.sa_handler = signal_debug,
	};

	mach->debug = 0;
	pthread_cond_init(&mach->debug_done, NULL);

	/*
	 * Prep our debug signal.
	 */
	sigemptyset(&saction.sa_mask);
	sigaction(SIGTSTP, &saction, NULL);

	/*
	 * Start the debgger thread; it'll idle until needed.
	 */
	r5sim_assert(0 ==
		     pthread_create(&mach->debug_thread, NULL,
				    debug_thread, mach));
}
