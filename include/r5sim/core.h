/*
 * RISC-V core abstraction.
 */

#ifndef __R5SIM_CORE_H__
#define __R5SIM_CORE_H__

#include <time.h>
#include <stdint.h>

#include <r5sim/isa.h>

struct r5sim_machine;
struct r5sim_core;
struct r5sim_csr;

typedef void (*r5sim_csr_fn)(struct r5sim_core *core,
			     struct r5sim_csr *csr);

struct r5sim_csr {
	uint32_t	value;
	uint32_t	flags;
#define CSR_PRESENT	0x1
#define CSR_READ	0x2
#define CSR_WRITE	0x4

	r5sim_csr_fn	read_fn;
	r5sim_csr_fn	write_fn;
};

#define CSR_CYCLE	0xC00
#define CSR_TIME	0xC01
#define CSR_INSTRET	0xC02
#define CSR_CYCLEH	0xC80
#define CSR_TIMEH	0xC81
#define CSR_INSTRETH	0xC82

#define r5sim_core_add_csr(core, __csr, __value, __flags)		\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_PRESENT,			\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)

#define r5sim_core_add_csr_fn(core, __csr, __value, __flags, rd, wr)	\
	do {								\
		struct r5sim_csr csr = {				\
			.value = __value,				\
			.flags = __flags | CSR_PRESENT,			\
			.read_fn = rd,					\
			.write_fn = wr,					\
		};							\
		__r5sim_core_add_csr(core, &csr, __csr);		\
	} while (0)


struct r5sim_core {
	const char           *name;

	uint32_t              reg_file[32];
	uint32_t              pc;

	struct r5sim_csr      csr_file[4096];

	struct timespec	      start;

	/*
	 * Machine this core belongs to; relevant for handling IO, DRAM
	 * access, etc.
	 */
	struct r5sim_machine *mach;

	/*
	 * Ask the core to execute _one_ instruction.
	 */
	int (*exec_one)(struct r5sim_machine *mach,
			struct r5sim_core *core);
};

static inline void
__set_reg(struct r5sim_core *core,
	  uint32_t reg, uint32_t val)
{
	r5sim_assert(reg < 32);

	/* Silently drop writes to x0 (zero). */
	if (reg == 0)
		return;

	core->reg_file[reg] = val;
}

static inline uint32_t
__get_reg(struct r5sim_core *core, uint32_t reg)
{
	r5sim_assert(reg < 32);

	return core->reg_file[reg];
}

static inline uint32_t
__raw_csr_read(struct r5sim_csr *csr)
{
	return csr->value;
}

static inline void
__raw_csr_write(struct r5sim_csr *csr, uint32_t value)
{
	csr->value = value;
}

static inline void
__raw_csr_set_mask(struct r5sim_csr *csr, uint32_t value)
{
	csr->value |= value;
}
static inline void
__raw_csr_clear_mask(struct r5sim_csr *csr, uint32_t value)
{
	csr->value &= ~value;
}

void
__csr_w(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr);
void
__csr_s(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr);
void
__csr_c(struct r5sim_core *core, uint32_t rd, uint32_t value, uint32_t csr);

void
__r5sim_core_add_csr(struct r5sim_core *core,
		     struct r5sim_csr *csr_reg,
		     uint32_t csr);

void
r5sim_core_init_common(struct r5sim_core *core);

void
r5sim_core_exec(struct r5sim_machine *mach,
		struct r5sim_core *core,
		uint32_t pc);

const char *
r5sim_reg_to_abi_str(uint32_t reg);
const char *
r5sim_reg_to_str(uint32_t reg);
const char *
r5sim_load_func3_to_str(uint32_t func3);
const char *
r5sim_store_func3_to_str(uint32_t func3);
const char *
r5sim_op_imm_func3_to_str(uint32_t func3);
const char *
r5sim_op_func3_to_str(uint32_t func3, uint32_t func7);
const char *
r5sim_branch_func3_to_str(uint32_t func3);
const char *
r5sim_system_func3_to_str(uint32_t func3);

void
r5sim_core_describe(struct r5sim_core *core);

#endif
