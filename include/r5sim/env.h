/*
 * Various environment functions that should be generally useful.
 */

#ifndef __R5SIM_ENV_H__
#define __R5SIM_ENV_H__

#include <assert.h>
#include <stdint.h>

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

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

#endif
