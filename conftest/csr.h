/*
 * CSRs accessors.
 */

#ifndef __CSR_H__
#define __CSR_H__

#include "types.h"

#include <r5sim/hw/csr.h>

#define read_csr(csr, value)			\
	asm volatile ("csrr	%0, %1\n\t"	\
		      : "=r" (value)		\
		      : "i" (csr))

#define write_csr(csr, value)			\
	asm volatile ("csrw	%0, %1\n\t"	\
		      :				\
		      : "i" (csr), "r" (value))

#define clear_csr(csr, value)			\
	asm volatile ("csrc	%0, %1\n\t"	\
		      :				\
		      : "i" (csr), "r" (value))

/*
 * Clear and then set a field to the passed value, __v.
 */
#define set_field(reg, __f, __v)			\
	({						\
		typeof(reg) v = __v << (0u ? __f);	\
		typeof(reg) m = (1u << (1u ? __f)) |	\
			((1u << (1u ? __f)) - 1u);	\
							\
		m ^= ((1u << (0u ? __f)) - 1u);		\
		reg &= ~m;				\
		reg |= v;				\
	})

#define get_field(reg, __f)				\
	({						\
		typeof(reg) m = (1u << (1u ? __f)) |	\
			((1u << (1u ? __f)) - 1u);	\
							\
		m ^= ((1u << (0u ? __f)) - 1u);		\
		(reg & m) >> (0u ? __f);		\
	})

#endif
