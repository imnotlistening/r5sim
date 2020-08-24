/*
 * Various environment functions that should be generally useful.
 */

#ifndef __R5SIM_ENV_H__
#define __R5SIM_ENV_H__

#include <assert.h>

/*
 * This may expand in the future.
 */
#define r5sim_assert(cond)	 assert(cond)

/*
 * Convenience macros for sizes.
 */
#define GB(x) ((x) << 30)
#define MB(x) ((x) << 20)
#define KB(x) ((x) << 10)

#endif
