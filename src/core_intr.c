/*
 * Manage interrupts pending on a core.
 *
 * The basic idea here is that when an external interrupt source
 * is ready to signal the core, it calls r5sim_core_intr_signal().
 * This tells the core that there's an itnerrutp pending.
 *
 * The core, after each instruction has executed, if interrupts
 * are not masked, checks if there's pending interrupts. If so
 * the oldest interrupt is popped and the core handles the
 * interrupt. A interrupt object is malloc()'ed by the
 * core_signal() function, so the core is expected to free() that
 * interrupt object when it's done with it.
 */

#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#include <r5sim/log.h>
#include <r5sim/core.h>

/*
 * Wait for an interrupt.
 */
void r5sim_core_wfi(struct r5sim_core *core)
{
	struct timespec spec = {
		.tv_sec  = 0,
		.tv_nsec = 500000, /* .5 milliseconds */
	};

	while (!r5sim_core_intr_pending(core))
		nanosleep(&spec, NULL);
}

int r5sim_core_intr_pending(struct r5sim_core *core)
{
	int pending;

	pthread_mutex_lock(&core->intr_lock);
	pending = !list_empty(&core->intrs);
	pthread_mutex_unlock(&core->intr_lock);

	return pending;
}

void r5sim_core_intr_signal(struct r5sim_core *core, u32 src)
{
	struct r5sim_core_interrupt *intr = malloc(sizeof(*intr));

	r5sim_assert(intr != NULL);
	intr->cause = src;

	r5sim_dbg("Interrupt reported: %u\n", src);

	pthread_mutex_lock(&core->intr_lock);
	list_add_tail(&intr->intr_node, &core->intrs);

	/*
	 * Adjust the MIP register to mark that this interrupt is
	 * pending.
	 */
	core->mip |= (1 << src);

	pthread_mutex_unlock(&core->intr_lock);
}

struct r5sim_core_interrupt *r5sim_core_intr_next(
	struct r5sim_core *core)
{
	struct r5sim_core_interrupt *intr = NULL;

	pthread_mutex_lock(&core->intr_lock);
	if (!list_empty(&core->intrs)) {
		intr = list_first_entry(&core->intrs,
					struct r5sim_core_interrupt,
					intr_node);
		list_del_init(&intr->intr_node);
	}
	pthread_mutex_unlock(&core->intr_lock);

	if (intr)
		r5sim_dbg("Interrupt dequeued!\n");

	return intr;
}
