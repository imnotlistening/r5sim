/*
 * Here's the logic for handling PMPs in the core. Generally all accesses
 * to memory for a given core go through the PMP.
 */

#include <strings.h>

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

static int pmp_is_active(struct r5sim_mmu *mmu)
{
	u32 i;

	for (i = 0; i < 16; i++) {
		if (mmu->configs[i].addr_match != PMPCFG_AM_OFF)
			return 1;
	}

	return 0;
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
	pmp_dbg("PMPADDR%-2d raw  0x%08x\n", pmpaddr_idx, entry->raw);
	pmp_dbg("PMPADDR%-2d size 0x%08x\n", pmpaddr_idx, entry->size);
	pmp_dbg("PMPADDR%-2d addr 0x%08x\n", pmpaddr_idx, entry->addr);
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

	pmp_dbg("Wrote PMPCFG%-2d | %-5s %c%c%c%c\n",
		cfg,
		addr_match_str(pmpcfg->addr_match),
		pmpcfg->locked ? 'l' : '-',
		pmpcfg->exec   ? 'x' : '-',
		pmpcfg->write  ? 'w' : '-',
		pmpcfg->read   ? 'r' : '-');
}

void pmpcfg_wr(struct r5sim_core *core,
	       struct r5sim_csr *csr,
	       u32 type, u32 *value)
{
	u32 pmpcfg_old, pmpcfg = 0;
	u32 csr_idx = r5sim_csr_index(core, csr);
	u32 pmpcfg_idx = csr_idx - CSR_PMPCFG0;

	pmpcfg = __raw_csr_read(csr);
	pmpcfg_old = pmpcfg;

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

	pmp_dbg("Updating PMPCFG%d; old value: 0x%08x\n",
		pmpcfg_idx, pmpcfg_old);

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
	core->mmu.pmp_active = pmp_is_active(&core->mmu);
	pmp_dbg("PMP active: %s\n", core->mmu.pmp_active ? "yes" : "no");
}

/*
 * Return 0 for no match, or non-zero for a match.
 */
static int pmp_check_match(struct r5sim_core *core,
			   u32 entry_nr, u32 addr)
{
	u32 base, end;
	struct pmpcfg *cfg = &core->mmu.configs[entry_nr];
	struct pmpentry *entry = &core->mmu.entries[entry_nr];

	/*
	 * Step one: determine a match. The largest access we support is
	 * 4 bytes and the min granularity of the PMP is 4 bytes so there
	 * cannot be any of the half in half out access considerations
	 * that the spec mentions. This will change if we support MXLEN=64.
	 * But in that case we'll probably just drop NA4 support.
	 */
	switch (cfg->addr_match) {
	case PMPCFG_AM_OFF:
		return 0; /* No matches. */
	case PMPCFG_AM_TOR:
		base = entry_nr ?
			core->mmu.entries[entry_nr - 1].raw : 0;
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

	pmp_trace("Check: 0x%08x in [0x%08x,0x%08x]\n",
		  addr, base, end);

	return addr >= base && addr < end;
}

/*
 * We have 3 possible returns:
 *
 *   -1  -  Matches and denied.
 *    0  -  No match.
 *    1  -  Matches and allowed.
 */
static int pmp_load_ok(struct r5sim_core *core,
		       u32 entry_nr, u32 addr)
{
	struct pmpcfg *cfg = &core->mmu.configs[entry_nr];

	if (!pmp_check_match(core, entry_nr, addr))
		return 0;

	/*
	 * We have a match - verify there's load permissions. We need to
	 * return -1 for a deny, 1 for an allow. Multplying by 2 gets us
	 * to either 0, or 2, depending on if read is set. Subtracting 1
	 * then gets to -1 or 1.
	 */
	if (!cfg->locked && core->priv > RV_PRIV_S)
		return 1;

	return (cfg->read << 1) - 1;
}

static int pmp_store_ok(struct r5sim_core *core,
			u32 entry_nr, u32 addr)
{
	struct pmpcfg *cfg = &core->mmu.configs[entry_nr];

	if (!pmp_check_match(core, entry_nr, addr))
		return 0;

	if (!cfg->locked && core->priv > RV_PRIV_S)
		return 1;

	return (cfg->write << 1) - 1;
}

static int pmp_exec_ok(struct r5sim_core *core,
		       u32 entry_nr, u32 addr)
{
	struct pmpcfg *cfg = &core->mmu.configs[entry_nr];

	if (!pmp_check_match(core, entry_nr, addr))
		return 0;

	if (!cfg->locked && core->priv > RV_PRIV_S)
		return 1;

	if (cfg->exec && cfg->read)
		return 1;
	return -1;
}

/*
 * Normally putting control flow logic in a macro is a bad idea. This is
 * also probably a bad idea... But... Is what it is.
 */
#define PMP_CHECK_AND_RETURN(func, core, entry, addr)		\
	do {							\
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
	} while (0)

/*
 * Check if the requested load is allowed. Return 0 if the load is allowed,
 * -1 if the load is rejected.
 */
int r5sim_pmp_load_allowed(struct r5sim_core *core, u32 addr)
{
	pmp_trace("PMP: LOAD  @ 0x%08x\n", addr);
	if (!core->mmu.pmp_active) {
		pmp_trace("PMP:    [--] allowed!\n");
		return 0;
	}

	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 0,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 1,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 2,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 3,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 4,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 5,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 6,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 7,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 8,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 9,  addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 10, addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 11, addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 12, addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 13, addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 14, addr);
	PMP_CHECK_AND_RETURN(pmp_load_ok, core, 15, addr);

	/* Nothing matched. M mode accesses are OK, sub-M not. */
	if (core->priv == RV_PRIV_M)
		return 0;
	return -1;
}

int r5sim_pmp_store_allowed(struct r5sim_core *core, u32 addr)
{
	pmp_trace("PMP: STORE @ 0x%08x\n", addr);
	if (!core->mmu.pmp_active) {
		pmp_trace("PMP:    [--] allowed!\n");
		return 0;
	}

	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 0,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 1,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 2,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 3,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 4,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 5,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 6,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 7,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 8,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 9,  addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 10, addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 11, addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 12, addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 13, addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 14, addr);
	PMP_CHECK_AND_RETURN(pmp_store_ok, core, 15, addr);

	if (core->priv == RV_PRIV_M)
		return 0;
	return -1;
}

int r5sim_pmp_exec_allowed(struct r5sim_core *core, u32 addr)
{
	pmp_trace("PMP: ILOAD @ 0x%08x\n", addr);
	if (!core->mmu.pmp_active) {
		pmp_trace("PMP:    [--] allowed!\n");
		return 0;
	}

	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 0,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 1,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 2,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 3,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 4,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 5,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 6,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 7,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 8,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 9,  addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 10, addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 11, addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 12, addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 13, addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 14, addr);
	PMP_CHECK_AND_RETURN(pmp_exec_ok, core, 15, addr);

	if (core->priv == RV_PRIV_M)
		return 0;
	return -1;
}
