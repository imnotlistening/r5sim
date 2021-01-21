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

#endif
