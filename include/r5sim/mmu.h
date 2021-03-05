/*
 * Implement a PMPs and an MMU per the RISC-V MM specs.
 */

#ifndef __R5SIM_MMU_H__
#define __R5SIM_MMU_H__

#include <r5sim/env.h>

struct r5sim_core;
struct r5sim_csr;

/*
 * Each PMPCFGx register has 4 configs in it, packed per byte.
 *
 * However, this isn't great for actual usage by the simulator since
 * we don't want to have to unpack that every load or store.
 *
 * Instead we'll keep the configs completely unpacked for actual
 * usage and only pack/unpack them on reads/writes.
 */
struct pmpcfg {
	u32 locked;
	u32 addr_match;
	u32 exec;
	u32 write;
	u32 read;
};

#define PMPCFG_AM_OFF		0
#define PMPCFG_AM_TOR		1
#define PMPCFG_AM_NA4		2
#define PMPCFG_AM_NAPOT		3

struct pmpentry {
	u32 raw;
	u32 addr;
	u32 size;
};

/*
 * Instead of traversing the PMP structs every instruction, precompile
 * the PMPs into a sequence of checks. The PMP structs are not that
 * complicated, but any if condition hurts. Also redundant loads of bases
 * and ends are all unnecessary.
 */
struct pmpcheck {
	/* Pulled and decoded from the ADDRx registers. */
	u32 base;
	u32 end;

	/* Pulled from the CFGx registers. */
	u32 locked;
	u32 exec;
	u32 write;
	u32 read;
};

struct r5sim_mmu {
	struct pmpcfg   configs[16];
	struct pmpentry entries[16];

	struct pmpcheck checks[16];

	int             pmp_active_checks;

	/*
	 * Load and store values via memory management hierarchy.
	 */
	int (*load8)(struct r5sim_mmu *mmu,
		     u32 addr, u8 *value);
	int (*load16)(struct r5sim_mmu *mmu,
		      u32 addr, u16 *value);
	int (*load32)(struct r5sim_mmu *mmu,
		      u32 addr, u32 *value);
	int (*iload)(struct r5sim_mmu *mmu,
		     u32 addr, u32 *value);
	int (*store8)(struct r5sim_mmu *mmu,
		      u32 addr, u8 value);
	int (*store16)(struct r5sim_mmu *mmu,
		       u32 addr, u16 value);
	int (*store32)(struct r5sim_mmu *mmu,
		       u32 addr, u32 value);
};

void r5sim_mmu_use_default(struct r5sim_core *core);

/*
 * For the access functions to verify accesses are OK.
 */
int r5sim_pmp_load_allowed(struct r5sim_core *core, u32 addr);
int r5sim_pmp_store_allowed(struct r5sim_core *core, u32 addr);
int r5sim_pmp_exec_allowed(struct r5sim_core *core, u32 addr);

/*
 * CSR interface.
 */
void pmpaddr_rd(struct r5sim_core *core,
		struct r5sim_csr *csr);
void pmpaddr_wr(struct r5sim_core *core,
		struct r5sim_csr *csr,
		u32 type, u32 *value);
void pmpcfg_rd(struct r5sim_core *core,
	       struct r5sim_csr *csr);
void pmpcfg_wr(struct r5sim_core *core,
	       struct r5sim_csr *csr,
	       u32 type, u32 *value);

#endif
