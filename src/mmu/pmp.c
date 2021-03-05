/*
 * Here's the logic for handling PMPs in the core. Generally all accesses
 * to memory for a given core go through the PMP.
 */

#include <string.h>

#include <r5sim/log.h>
#include <r5sim/mmu.h>
#include <r5sim/csr.h>
#include <r5sim/core.h>
#include <r5sim/util.h>

#define pmp_dbg   r5sim_dbg_v
#define pmp_trace r5sim_dbg_vv

const char *addr_match_str(u32 addr_match)
{
	switch (addr_match) {
	case 0:
		return "OFF";
	case 1:
		return "TOR";
	case 2:
		return "NA4";
	case 3:
		return "NAPOT";
	}

	return "INVAL";
}

/*
 * From the PMP registers compile the list of checks we must do and specify
 * the number.
 *
 * This compiles the checks into a condensed list. Now there'll be no need
 * to iterate through the entire list of PMPs; it's a lot slower for SW to
 * do this than HW.
 */
static void pmp_compile(struct r5sim_mmu *mmu)
{
	u32 i;
	u32 base, end;
	struct pmpcfg *cfg;
	struct pmpentry *entry;
	struct pmpcheck *check;

	memset(mmu->checks, 0, sizeof(mmu->checks));
	mmu->pmp_active_checks = 0;
	check = &mmu->checks[0];

	for (i = 0; i < 16; i++) {
		cfg = &mmu->configs[i];
		entry = &mmu->entries[i];

		switch (cfg->addr_match) {
		case PMPCFG_AM_OFF:
			continue;
		case PMPCFG_AM_TOR:
			base = i ? mmu->entries[i - 1].raw : 0;
			base <<= 2;
			end  = entry->addr;
			break;
		case PMPCFG_AM_NA4:
			base = entry->addr;
			end  = base + 4;
			break;
		case PMPCFG_AM_NAPOT:
			base = entry->addr;
			end  = base + entry->size;
			break;
		}

		check->base   = base;
		check->end    = end;
		check->locked = cfg->locked;
		check->exec   = cfg->exec;
		check->write  = cfg->write;
		check->read   = cfg->read;

		/*
		 * Debugging! It's important.
		 */
		pmp_dbg("PMP %-2d | %s [%c%c%c%c] 0x%08x -> 0x%08x\n",
			i,
			cfg->addr_match == PMPCFG_AM_TOR   ? "TOR  " :
			cfg->addr_match == PMPCFG_AM_NA4   ? "NA4  " :
			cfg->addr_match == PMPCFG_AM_NAPOT ? "NAPOT" : NULL,
			check->locked ? 'l' : '-',
			check->exec   ? 'x' : '-',
			check->write  ? 'w' : '-',
			check->read   ? 'r' : '-',
			check->base, check->end);

		check++;
		mmu->pmp_active_checks++;
	}
}

/*
 * For the given address CSR, pack in struct pmpentry to the backing CSR.
 */
void pmpaddr_rd(struct r5sim_core *core,
		struct r5sim_csr *csr)
{
	u32 csr_idx = r5sim_csr_index(core, csr);
	u32 pmpaddr_idx = csr_idx - CSR_PMPADDR0;
	struct pmpentry *entry;

	r5sim_assert(pmpaddr_idx < 16);
	entry = &core->mmu.entries[pmpaddr_idx];

	__raw_csr_write(csr, entry->raw);
}

void pmpaddr_wr(struct r5sim_core *core,
		struct r5sim_csr *csr,
		u32 type, u32 *value)
{
	u32 csr_idx = r5sim_csr_index(core, csr);
	u32 pmpaddr_idx = csr_idx - CSR_PMPADDR0;
	struct pmpentry *entry;


	r5sim_assert(pmpaddr_idx < 16);
	entry = &core->mmu.entries[pmpaddr_idx];

	/*
	 * First we'll write the requested bits, per the specified operation.
	 * Then we'll unpack the bits into the relevant pmpentry struct. It
	 * may be, depending on the relevant cfg, that we don't use the unpacked
	 * version of the address.
	 */
	switch (type) {
	case CSR_WRITE:
		entry->raw = *value;
		break;
	case CSR_SET:
		entry->raw |= *value;
		break;
	case CSR_CLR:
		entry->raw &= ~(*value);
		break;
	}

	/* Unpack. */
	entry->size = 1 << (ffs(~entry->raw) + 2);
	entry->addr = (entry->raw << 2) & ~(entry->size - 1);
}

static u32 __pack_one_cfg(struct r5sim_core *core,
			  u32 cfg)
{
	struct pmpcfg *pmpcfg;
	u32 config_value = 0;

	r5sim_assert(cfg < 16);
	pmpcfg = &core->mmu.configs[cfg];

	set_field(config_value, CSR_PMPCFG_L, pmpcfg->locked);
	set_field(config_value, CSR_PMPCFG_A, pmpcfg->addr_match);
	set_field(config_value, CSR_PMPCFG_X, pmpcfg->exec);
	set_field(config_value, CSR_PMPCFG_W, pmpcfg->write);
	set_field(config_value, CSR_PMPCFG_R, pmpcfg->read);

	return config_value;
}

/*
 * Offset is [0..3] and represents a specific CFG register; we'll
 * pack configs {offset * 4, ..., offset * 4 + 3} and return that.
 */
static u32 __pack_cfgs(struct r5sim_core *core,
		       u32 offset)
{
	u32 pmpcfg = 0;
	u32 cfgoffs = offset * 4;

	pmpcfg |= __pack_one_cfg(core, cfgoffs);
	pmpcfg |= __pack_one_cfg(core, cfgoffs + 1) << 8;
	pmpcfg |= __pack_one_cfg(core, cfgoffs + 2) << 16;
	pmpcfg |= __pack_one_cfg(core, cfgoffs + 3) << 24;

	return pmpcfg;
}

void pmpcfg_rd(struct r5sim_core *core,
	       struct r5sim_csr *csr)
{
	u32 csr_idx = r5sim_csr_index(core, csr);
	u32 pmpcfg_idx = csr_idx - CSR_PMPCFG0;

	__raw_csr_write(csr, __pack_cfgs(core, pmpcfg_idx));
}

/*
 * Unpacks the config and returns the unpacked value; this takes into
 * account the lock bit.
 */
static void __unpack_one_cfg(struct r5sim_core *core,
			     u32 cfg, u32 value)
{
	struct pmpcfg *pmpcfg;

	r5sim_assert(cfg < 16);
	pmpcfg = &core->mmu.configs[cfg];

	/*
	 * No writes to locked configs!
	 */
	if (pmpcfg->locked)
		return;

	pmpcfg->locked     = get_field(value, CSR_PMPCFG_L);
	pmpcfg->addr_match = get_field(value, CSR_PMPCFG_A);
	pmpcfg->exec       = get_field(value, CSR_PMPCFG_X);
	pmpcfg->write      = get_field(value, CSR_PMPCFG_W);
	pmpcfg->read       = get_field(value, CSR_PMPCFG_R);
}

void pmpcfg_wr(struct r5sim_core *core,
	       struct r5sim_csr *csr,
	       u32 type, u32 *value)
{
	u32 pmpcfg = 0;
	u32 csr_idx = r5sim_csr_index(core, csr);
	u32 pmpcfg_idx = csr_idx - CSR_PMPCFG0;

	pmpcfg = __raw_csr_read(csr);

	/*
	 * We'll attempt to update the pmpcfg based on the passed write.
	 * However, once we check the lock bits, we may end up filtering
	 * out some writes.
	 */
	switch (type) {
	case CSR_WRITE:
		pmpcfg = *value;
		break;
	case CSR_SET:
		pmpcfg |= *value;
		break;
	case CSR_CLR:
		pmpcfg &= ~(*value);
		break;
	}

	/* Scale the config index since it came in as a multiple of 4. */
	pmpcfg_idx *= 4;

	/*
	 * Handle writes to the actual PMPCFGs we use in the core.
	 */
	__unpack_one_cfg(core, pmpcfg_idx + 0,  pmpcfg        & 0xff);
	__unpack_one_cfg(core, pmpcfg_idx + 1, (pmpcfg >> 8)  & 0xff);
	__unpack_one_cfg(core, pmpcfg_idx + 2, (pmpcfg >> 16) & 0xff);
	__unpack_one_cfg(core, pmpcfg_idx + 3, (pmpcfg >> 24) & 0xff);

	/*
	 * Now, to get the value to actually write we'll just repack what we
	 * unpacked. We write this directly to the CSR and ask the core CSR
	 * code not to do this.
	 */
	__raw_csr_write(csr, __pack_cfgs(core, pmpcfg_idx >> 2));

	/*
	 * See if any PMPs are active; if no PMPs are active then we don't
	 * reject U/S accesses if there's no match. If at least one PMP is
	 * active, then we will reject U/S accesses if there's no match.
	 */
	pmp_compile(&core->mmu);
	pmp_dbg("PMP active: %s\n",
		core->mmu.pmp_active_checks ? "yes" : "no");
}

/*
 * We have 3 possible returns:
 *
 *   -1   -  Matches and rejected.
 *    0   -  No match.
 *    1   -  Matches and allowed.
 */
static int pmp_load_ok(struct r5sim_core *core,
		       u32 entry_nr, u32 addr)
{
	struct pmpcheck *check = &core->mmu.checks[entry_nr];
	int addr_check;
	int priv_allowed;
	int load_allowed;

	addr_check   = addr >= check->base && addr < check->end;
	priv_allowed = !check->locked && core->priv > RV_PRIV_S;
	load_allowed = check->read || priv_allowed;

	/*
	 * A dirty optimization. This completely avoids the need for branches
	 * in the PMP check here. The branching is at a level higher.
	 *
	 * This takes advantage of the fact that C gives us back a 1 or 0 for
	 * a comparison.
	 *
	 * If addr_check is 0 we are done. 0 anded with anything is just 0
	 * and since the PMP didn't match we want to return 0. The -addr_check
	 * stays 0 if it's already 0, or becomes -1 (i.e 0xffffffff) if it's 1
	 * and thus lets the subsequent condition through.
	 *
	 * If we do have a match we want to return -1 for a reject or 1 for an
	 * allow. load_allowed will be 1 if either we have the priv override or
	 * if the PMP allows the sub-M mode SW to access it. We then scale by
	 * 2 to either 0 or 2 and then subtract 1. That way we return -1 when
	 * load_allowed is 0, or 1 when load_allowed is 1.
	 */
	return -addr_check & ((load_allowed << 1) - 1);
}

static int pmp_store_ok(struct r5sim_core *core,
			u32 entry_nr, u32 addr)
{
	struct pmpcheck *check = &core->mmu.checks[entry_nr];
	int addr_check;
	int priv_allowed;
	int store_allowed;

	addr_check    = addr >= check->base && addr < check->end;
	priv_allowed  = !check->locked && core->priv > RV_PRIV_S;
	store_allowed = check->read || priv_allowed;

	return -addr_check & ((store_allowed << 1) - 1);
}

static int pmp_exec_ok(struct r5sim_core *core,
		       u32 entry_nr, u32 addr)
{
	struct pmpcheck *check = &core->mmu.checks[entry_nr];
	int addr_check;
	int priv_allowed;
	int exec_allowed;

	addr_check   = addr >= check->base && addr < check->end;
	priv_allowed = !check->locked && core->priv > RV_PRIV_S;
	exec_allowed = (check->exec && check->read) || priv_allowed;

	return -addr_check & ((exec_allowed << 1) - 1);
}

/*
 * Normally putting control flow logic in a macro is a bad idea. This is
 * also probably a bad idea... But... Is what it is.
 */
#define PMP_CHECK_AND_RETURN(func, core, entry, addr)		\
	{							\
		int ok = func(core, entry, addr);		\
								\
		switch (ok) {					\
		case -1: /* Match and reject; */		\
			pmp_trace("PMP:   [%02d] rejected!\n",	\
				  entry);			\
			return -1;				\
		case 0:	 /* No match; */			\
			pmp_trace("PMP:   [%02d] (next)\n",	\
				  entry);			\
			break;					\
		case 1:	 /* Match and allow. */			\
			pmp_trace("PMP:   [%02d] allowed!\n\n",	\
				  entry);			\
			return 0;				\
		}						\
	}

/*
 * Either nothing matched or there were no PMPs. If there's no PMPs
 * installed, then every access is allowed. If there is at least one
 * PMP installed, but no match, M-Mode accesses are still allowed.
 *
 * If neither case is true, then return -1; otherwise return 0.
 */
#define PMP_NO_MATCH_CHECK(core)					\
	(((core)->priv == RV_PRIV_M ||					\
	  (core)->mmu.pmp_active_checks == 0) - 1)

/*
 * Check if the requested load is allowed. Return 0 if the load is allowed,
 * -1 if the load is rejected.
 */
int r5sim_pmp_load_allowed(struct r5sim_core *core, u32 addr)
{
	int i = 0;

	pmp_trace("PMP: LOAD  @ 0x%08x\n", addr);

	for (i = 0; i < core->mmu.pmp_active_checks; i++)
		PMP_CHECK_AND_RETURN(pmp_load_ok, core, i, addr);

	return PMP_NO_MATCH_CHECK(core);
}

int r5sim_pmp_store_allowed(struct r5sim_core *core, u32 addr)
{
	int i = 0;

	pmp_trace("PMP: LOAD  @ 0x%08x\n", addr);

	for (i = 0; i < core->mmu.pmp_active_checks; i++)
		PMP_CHECK_AND_RETURN(pmp_store_ok, core, i, addr);

	/* Nothing matched. M mode accesses are OK, sub-M not. */
	return PMP_NO_MATCH_CHECK(core);
}

int r5sim_pmp_exec_allowed(struct r5sim_core *core, u32 addr)
{
	int i = 0;

	pmp_trace("PMP: LOAD  @ 0x%08x\n", addr);

	for (i = 0; i < core->mmu.pmp_active_checks; i++)
		PMP_CHECK_AND_RETURN(pmp_exec_ok, core, i, addr);

	/* Nothing matched. M mode accesses are OK, sub-M not. */
	return PMP_NO_MATCH_CHECK(core);
}
