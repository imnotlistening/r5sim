/*
 * General utilities useful to r5sim.
 */

#ifndef __R5SIM_UTIL_H__
#define __R5SIM_UTIL_H__

static inline uint32_t sign_extend(uint32_t r, uint32_t sbit)
{
	uint32_t m = 1 << sbit;

	return (r ^ m) - m;
}

#define addr_in(base, length, addr)		\
	(((addr) >= (base)) &&			\
	 ((addr) < ((base) + (length))))


#define max(a, b)				\
	({ __typeof__ (a) __a = (a);		\
	   __typeof__ (b) __b = (b);		\
	   __a > __b ? __a : __b; })
#define min(a, b)				\
	({ __typeof__ (a) __a = (a);		\
	   __typeof__ (b) __b = (b);		\
	   __a < __b ? __a : __b; })


#endif
