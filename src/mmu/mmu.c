/*
 * General MMU interaction or a core.
 */

#include <r5sim/mmu.h>
#include <r5sim/core.h>
#include <r5sim/util.h>
#include <r5sim/machine.h>

#define mmu_to_core(mmu)					\
	container_of(mmu, struct r5sim_core, mmu)
#define mmu_to_mach(mmu)					\
	(container_of(mmu, struct r5sim_core, mmu)->mach)

int r5sim_default_load8(struct r5sim_mmu *mmu,
			u32 addr, u8 *value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_load_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memload8(mach, addr, value);
}

int r5sim_default_load16(struct r5sim_mmu *mmu,
			 u32 addr, u16 *value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_load_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memload16(mach, addr, value);
}

int r5sim_default_load32(struct r5sim_mmu *mmu,
			 u32 addr, u32 *value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_load_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memload32(mach, addr, value);
}

int r5sim_default_iload(struct r5sim_mmu *mmu,
			u32 addr, u32 *value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_exec_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memload32(mach, addr, value);
}

int r5sim_default_store8(struct r5sim_mmu *mmu,
			 u32 addr, u8 value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_store_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memstore8(mach, addr, value);
}

int r5sim_default_store16(struct r5sim_mmu *mmu,
			  u32 addr, u16 value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_store_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memstore16(mach, addr, value);
}

int r5sim_default_store32(struct r5sim_mmu *mmu,
			  u32 addr, u32 value)
{
	struct r5sim_machine *mach = mmu_to_mach(mmu);

	if (r5sim_pmp_store_allowed(mmu_to_core(mmu), addr))
		return __ACCESS_FAULT;

	return mach->memstore32(mach, addr, value);
}

void r5sim_mmu_use_default(struct r5sim_core *core)
{
	core->mmu.load8   = r5sim_default_load8;
	core->mmu.load16  = r5sim_default_load16;
	core->mmu.load32  = r5sim_default_load32;
	core->mmu.iload   = r5sim_default_iload;
	core->mmu.store8  = r5sim_default_store8;
	core->mmu.store16 = r5sim_default_store16;
	core->mmu.store32 = r5sim_default_store32;
}
