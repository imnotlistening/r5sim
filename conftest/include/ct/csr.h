/* Copyright 2021, Alex Waterman <imnotlistening@gmail.com>
 *
 * This file is part of r5sim.
 *
 * r5sim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * r5sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with r5sim.  If not, see <https://www.gnu.org/licenses/>.
 *
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
