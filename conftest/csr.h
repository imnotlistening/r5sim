/*
 * CSRs accessors.
 */

#ifndef __CSR_H__
#define __CSR_H__

#include "types.h"

#define read_csr(csr, value)			\
	asm volatile ("csrr	%0, %1\n\t"	\
		      : "=r" (value)		\
		      : "i" (csr))

#define write_csr(csr, value)			\
	asm volatile ("csrw	%0, %1\n\t"	\
		      :				\
		      : "i" (csr), "r" (value))

#define CSR_MISA		0x301
#define CSR_MISA_MXL		31:30
#define CSR_MISA_EXTENSION	25:0

/*
 * Clear and then set a field to the passed value, __v.
 */
#define set_field(reg, __f, __v)			\
	({						\
		typeof(reg) v = __v << (0 ? __f);	\
		typeof(reg) m = (1 << (1 ? __f)) |	\
			((1 << (1 ? __f)) - 1u);	\
							\
		m ^= ((1 << (0 ? __f)) - 1);		\
		v &= m;					\
		reg |= v;				\
	})

#define get_field(reg, __f)				\
	({						\
		typeof(reg) m = (1 << (1 ? __f)) |	\
			((1 << (1 ? __f)) - 1u);	\
							\
		m ^= ((1 << (0 ? __f)) - 1);		\
		(reg & m) >> (0 ? __f);			\
	})

#endif
