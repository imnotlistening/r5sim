/*
 * General utilities useful to r5sim.
 */

#ifndef __R5SIM_UTIL_H__
#define __R5SIM_UTIL_H__

#include <r5sim/env.h>

static inline u32 sign_extend(u32 r, u32 sbit)
{
	u32 m = 1 << sbit;

	return (r ^ m) - m;
}

static inline uint64_t sign_extend_64(uint64_t r, uint64_t sbit)
{
	uint64_t m = 1 << sbit;

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

#define container_of(ptr, type, member)					\
	({								\
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );	\
	})

/*
 * Force the compiler to do this read.
 */
#define __VOL_READ(x)					\
	({ volatile typeof(x) *__x = &x; *__x; })

/*
 * Clear and then set a field to the passed value, __v.
 */
#define	set_field(reg, __f, __v)			\
	({						\
		typeof(reg) v = __v << (0u ? __f);	\
		typeof(reg) m = (1u << (1u ? __f)) |	\
			((1u << (1u ? __f)) - 1u);	\
							\
		m ^= ((1u << (0u ? __f)) - 1u);		\
		reg &= ~m;				\
		reg |= v;				\
	})

#define	get_field(reg, __f)				\
	({						\
		typeof(reg) m = (1u << (1u ? __f)) |	\
			((1u << (1u ? __f)) - 1u);	\
							\
		m ^= ((1u << (0u ? __f)) - 1u);		\
		(reg & m) >> (0u ? __f);		\
	})

#endif
