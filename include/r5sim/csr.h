/*
 * RISC-V CSR definitions, etc.
 */

#ifndef __R5SIM_CSR_H__
#define __R5SIM_CSR_H__

#include <stdint.h>

struct r5sim_core;
struct r5sim_csr;

typedef void (*r5sim_csr_fn)(struct r5sim_core *core,
			     struct r5sim_csr *csr);

struct r5sim_csr {
	u32	value;
	u32	flags;
#define CSR_F_PRESENT	0x1
#define CSR_F_READ	0x2
#define CSR_F_WRITE	0x4

	r5sim_csr_fn	read_fn;
	r5sim_csr_fn	write_fn;

	u32	wpri_mask;
	u32	wlrl_mask;
	u32	warl_mask;
};

/*
 * Basic CSRs.
 */
#define CSR_CYCLE		0xC00
#define CSR_TIME		0xC01
#define CSR_INSTRET		0xC02
#define CSR_CYCLEH		0xC80
#define CSR_TIMEH		0xC81
#define CSR_INSTRETH		0xC82

/*
 * Machine mode CSRs
 */
#define CSR_MSTATUS		0x300
#define CSR_MISA		0x301
#define CSR_MEDELEG		0x302
#define CSR_MIDELEG		0x303
#define CSR_MIE			0x304

#define CSR_MTVEC		0x305
#define CSR_MTVEC_BASE		31:2
#define CSR_MTVEC_MODE		1:0

#define CSR_MSCRATCH		0x340
#define CSR_MEPC		0x341
#define CSR_MCAUSE		0x342
#define CSR_MCAUSE_INTERRUPT	31:31
#define CSR_MCAUSE_CODE		30:0

#define CSR_MTVAL		0x343
#define CSR_MIP			0x344

#define CSR_MVENDORID		0xF11
#define CSR_MARCHID		0xF21
#define CSR_MIMPID		0xF13
#define CSR_MHARTID		0xF14

#define r5sim_core_add_csr(core, __csr, __value, __flags)		\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,			\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

#define r5sim_core_add_csr_fn(core, __csr, __value, __flags, rd, wr)	\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_F_PRESENT,			\
			.read_fn = rd,					\
			.write_fn = wr,					\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

u32
csr_read(struct r5sim_core *core, u32 csr);
void
csr_write(struct r5sim_core *core, u32 csr, u32 value);

static inline u32
__raw_csr_read(struct r5sim_csr *csr)
{
	return csr->value;
}

static inline void
__raw_csr_write(struct r5sim_csr *csr, u32 value)
{
	csr->value = value;
}

static inline void
__raw_csr_set_mask(struct r5sim_csr *csr, u32 value)
{
	csr->value |= value;
}

static inline void
__raw_csr_clear_mask(struct r5sim_csr *csr, u32 value)
{
	csr->value &= ~value;
}

void
__csr_w(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
void
__csr_s(struct r5sim_core *core, u32 rd, u32 value, u32 csr);
void
__csr_c(struct r5sim_core *core, u32 rd, u32 value, u32 csr);

void
__r5sim_core_add_csr(struct r5sim_core *core,
		     struct r5sim_csr *csr_reg,
		     u32 csr);

#endif
