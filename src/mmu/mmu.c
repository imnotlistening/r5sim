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
