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

/*
 * Clear and then set a field to the passed value, __v.
 */
#define	set_field(reg, __f, __v)			\
	({						\
		typeof(reg) v = __v << (0 ? __f);	\
		typeof(reg) m = (1 << (1 ? __f)) |	\
			((1 << (1 ? __f)) - 1u);	\
							\
		m ^= ((1 << (0 ? __f)) - 1);		\
		v &= m;					\
		reg |= v;				\
	})

#define	get_field(reg, __f)				\
	({						\
		typeof(reg) m = (1 << (1 ? __f)) |	\
			((1 << (1 ? __f)) - 1u);	\
							\
		m ^= ((1 << (0 ? __f)) - 1);		\
		(reg & m) >> (0 ? __f);			\
	})

#endif
