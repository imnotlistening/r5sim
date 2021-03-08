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
 * RISC-V CSR definitions, etc.
 */

#ifndef __R5SIM_CSR_H__
#define __R5SIM_CSR_H__

#include <r5sim/env.h>

#include <r5sim/hw/csr.h>

struct r5sim_core;
struct r5sim_csr;

typedef void (*r5sim_csr_rdfn)(struct r5sim_core *core,
			       struct r5sim_csr *csr);
typedef void (*r5sim_csr_wrfn)(struct r5sim_core *core,
			       struct r5sim_csr *csr,
			       u32 type, u32 *value);

#define CSR_WRITE	0
#define CSR_SET		1
#define CSR_CLR		2

struct r5sim_csr {
	u32	value;
	u32	flags;
#define CSR_F_PRESENT		0x1
#define CSR_F_READ		0x2
#define CSR_F_WRITE		0x4
#define CSR_F_SKIP_WRITE	0x8

	/*
	 * Use these functions to intercept reads and writes. You can
	 * either modify incoming writes or create the out going reads.
	 */
	r5sim_csr_rdfn	read_fn;
	r5sim_csr_wrfn	write_fn;

	const char *name;
};

#define r5sim_core_add_csr(core, __csr, __value, __flags)		\
	do {								\
		struct r5sim_csr csr = {				\
			.name = #__csr,					\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,		\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

#define r5sim_core_add_csr_fn(core, __csr, __value, __flags, rd, wr)	\
	do {								\
		struct r5sim_csr csr = {				\
			.name = #__csr,					\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,		\
			.read_fn = rd,					\
			.write_fn = wr,					\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

u32  csr_read(struct r5sim_core *core, u32 csr);
void csr_write(struct r5sim_core *core, u32 csr, u32 value);
u32  r5sim_csr_index(struct r5sim_core *core,
		     struct r5sim_csr *csr);

static inline u32 __raw_csr_read(struct r5sim_csr *csr)
{
	return csr->value;
}

static inline void __raw_csr_write(struct r5sim_csr *csr, u32 value)
{
	csr->value = value;
}

static inline void __raw_csr_set_mask(struct r5sim_csr *csr,
				      u32 value)
{
	csr->value |= value;
}

static inline void __raw_csr_clear_mask(struct r5sim_csr *csr,
					u32 value)
{
	csr->value &= ~value;
}

int __csr_w(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
int __csr_s(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
int __csr_c(struct r5sim_core *core, u32 rd, u32 value, u32 csr);

void __r5sim_core_add_csr(struct r5sim_core *core,
			  struct r5sim_csr *csr_reg,
			  u32 csr);
void r5sim_core_default_csrs(struct r5sim_core *core);

/*
 * The CSR debug code uses this; it's a bit awkward. Don't use this
 * unless you know what you are doing!
 */
struct r5sim_csr *__csr_always(struct r5sim_core *core, u32 rd, u32 csr);

/* For use with get_field(). */
#define CSR_PRIV_FIELD			9:8

#endif
