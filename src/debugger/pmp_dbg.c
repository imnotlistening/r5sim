/*
 * Display the active PMPs.
 *
 * Usage
 *
 *   $ pmp
 */

#include <stdio.h>

#include <r5sim/csr.h>
#include <r5sim/mmu.h>
#include <r5sim/core.h>
#include <r5sim/hwdebug.h>
#include <r5sim/machine.h>

static void usage(void)
{
	printf("Usage:\n"
	       "\n"
	       "  $ pmp\n"
	       "\n"
	       "Displays active PMP configurations.\n\n"
		);
}

static void print_one_pmp(struct r5sim_mmu *mmu, u32 i)
{
	struct pmpcfg *cfg = &mmu->configs[i];
	struct pmpentry *entry = &mmu->entries[i];
	u32 base, end;

	switch (cfg->addr_match) {
	case PMPCFG_AM_OFF:
		printf("PMP %-2d | OFF\n", i);
		return;
	case PMPCFG_AM_TOR:
		printf("PMP %-2d | TOR   ", i);
		base = i ? mmu->entries[i - 1].raw : 0;
		base <<= 2;
		end  = entry->addr;
		break;
	case PMPCFG_AM_NA4:
		printf("PMP %-2d | NA4   ", i);
		base = entry->addr;
		end  = base + 4;
		break;
	case PMPCFG_AM_NAPOT:
		printf("PMP %-2d | NAPO2 ", i);
		base = entry->addr;
		end  = base + entry->size;
		break;
	}

	printf("[%c%c%c%c] ",
	       cfg->locked ? 'l' : '-',
	       cfg->exec   ? 'x' : '-',
	       cfg->write  ? 'w' : '-',
	       cfg->read   ? 'r' : '-');

	printf(" 0x%08x -> 0x%08x\n", base, end);
}

static void print_pmps(struct r5sim_machine *mach)
{
	struct r5sim_mmu *mmu = &mach->core->mmu;
	u32 i;

	for (i = 0; i < 16; i++)
		print_one_pmp(mmu, i);
}

int comm_pmp(struct r5sim_machine *mach, int argc, char *argv[])
{
	if (argc != 1) {
		usage();
		return -1;
	}

	print_pmps(mach);

	return 0;
}
