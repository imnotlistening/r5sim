/*
 * RISC-V CSR definitions, etc.
 */

#ifndef __R5SIM_CSR_H__
#define __R5SIM_CSR_H__

#include <stdint.h>

#include <r5sim/hw/csr.h>

struct r5sim_core;
struct r5sim_csr;

typedef void (*r5sim_csr_rdfn)(struct r5sim_core *core,
			       struct r5sim_csr *csr);
typedef void (*r5sim_csr_wrfn)(struct r5sim_core *core,
			       struct r5sim_csr *csr,
			       u32 *value);

struct r5sim_csr {
	u32	value;
	u32	flags;
#define CSR_F_PRESENT	0x1
#define CSR_F_READ	0x2
#define CSR_F_WRITE	0x4

	/*
	 * Use these functions to intercept reads and writes. You can
	 * either modify incoming writes or create the out going reads.
	 */
	r5sim_csr_rdfn	read_fn;
	r5sim_csr_wrfn	write_fn;
};

#define r5sim_core_add_csr(core, __csr, __value, __flags)		\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,		\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

#define r5sim_core_add_csr_fn(core, __csr, __value, __flags, rd, wr)	\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,		\
			.read_fn = rd,					\
			.write_fn = wr,					\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

u32  csr_read(struct r5sim_core *core, u32 csr);
void csr_write(struct r5sim_core *core, u32 csr, u32 value);

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

void __csr_w(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
void __csr_s(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
void __csr_c(struct r5sim_core *core, u32 rd, u32 value, u32 csr);

void __r5sim_core_add_csr(struct r5sim_core *core,
			  struct r5sim_csr *csr_reg,
			  u32 csr);
void r5sim_core_default_csrs(struct r5sim_core *core);

#endif
