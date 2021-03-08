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
 * Basic CSR implementation.
 */

#include <stdlib.h>

#include <time.h>

#include <r5sim/env.h>
#include <r5sim/log.h>
#include <r5sim/csr.h>
#include <r5sim/mmu.h>
#include <r5sim/core.h>
#include <r5sim/util.h>

/*
 * Any write to this triggers an immediate simulator exit.
 */
static void csr_sim_exit(struct r5sim_core *core,
			 struct r5sim_csr *csr,
			 u32 type, u32 *value)
{
	exit(*value);
}

static void csr_mstatus_read(struct r5sim_core *core,
			     struct r5sim_csr *csr)
{
	__raw_csr_write(&core->csr_file[CSR_MSTATUS], core->mstatus);
}

static void csr_mstatus_write(struct r5sim_core *core,
			      struct r5sim_csr *csr,
			      u32 type, u32 *value)
{
	/*
	 * We only support a few fields in MSTATUS at the moment.
	 */
	const u32 mstatus_mask = 0x1baa;

	*value &= mstatus_mask;

	switch (type) {
	case CSR_WRITE:
		core->mstatus = *value;
		break;
	case CSR_SET:
		core->mstatus |= *value;
		break;
	case CSR_CLR:
		core->mstatus &= ~(*value);
		break;
	}
}

static void csr_medeleg_write(struct r5sim_core *core,
			      struct r5sim_csr *csr,
			      u32 type, u32 *value)
{
	*value &= 0xffff;

	switch (type) {
	case CSR_WRITE:
		core->medeleg = *value;
		break;
	case CSR_SET:
		core->medeleg |= *value;
		break;
	case CSR_CLR:
		core->medeleg &= ~(*value);
		break;
	}
}

static void csr_mideleg_write(struct r5sim_core *core,
			      struct r5sim_csr *csr,
			      u32 type, u32 *value)
{
	*value &= 0xaa;

	switch (type) {
	case CSR_WRITE:
		core->mideleg = *value;
		break;
	case CSR_SET:
		core->mideleg |= *value;
		break;
	case CSR_CLR:
		core->mideleg &= ~(*value);
		break;
	}
}

static void csr_mie_read(struct r5sim_core *core,
			 struct r5sim_csr *csr)
{
	__raw_csr_write(&core->csr_file[CSR_MIE], core->mie);
}

static void csr_mie_write(struct r5sim_core *core,
			  struct r5sim_csr *csr,
			  u32 type, u32 *value)
{
	/*
	 * We only support the [MS]SI/[MS]TI interrupts atm.
	 */
	const u32 mie_mask = 0xaa;

	*value &= mie_mask;

	switch (type) {
	case CSR_WRITE:
		core->mie = *value;
		break;
	case CSR_SET:
		core->mie |= *value;
		break;
	case CSR_CLR:
		core->mie &= ~(*value);
		break;
	}
}

static void csr_mip_read(struct r5sim_core *core,
			 struct r5sim_csr *csr)
{
	__raw_csr_write(&core->csr_file[CSR_MIP], core->mip);
}

static void csr_mip_write(struct r5sim_core *core,
			  struct r5sim_csr *csr,
			  u32 type, u32 *value)
{
	/*
	 * We only support the [MS]SI/[MS]TI interrupts atm.
	 */
	const u32 mip_mask = 0xaa;

	*value &= mip_mask;

	switch (type) {
	case CSR_WRITE:
		core->mip = *value;
		break;
	case CSR_SET:
		core->mip |= *value;
		break;
	case CSR_CLR:
		core->mip &= ~(*value);
		break;
	}
}

static void csr_sstatus_read(struct r5sim_core *core,
			     struct r5sim_csr *csr)
{
	/*
	 * SSTATUS is a shadow of mstatus so just mask off the MSTATUS
	 * bits we don't want to leak to supervisor mode.
	 */
	const u32 sstatus_mask = 0x122;

	__raw_csr_write(&core->csr_file[CSR_MSTATUS],
			core->mstatus & sstatus_mask);
}

static void csr_sstatus_write(struct r5sim_core *core,
			      struct r5sim_csr *csr,
			      u32 type, u32 *value)
{
	const u32 sstatus_mask = 0x122;
	u32 tmp_status;

	*value &= sstatus_mask;

	switch (type) {
	case CSR_WRITE:
		/*
		 * We only want supervisors to have access to bits in the
		 * sstatus_mask. So preserve everything else in mstatus,
		 * then let supervisors program the sstatus bits.
		 */
		tmp_status = core->mstatus & ~sstatus_mask;
		core->mstatus = tmp_status | *value;
		break;
	case CSR_SET:
		core->mstatus |= *value;
		break;
	case CSR_CLR:
		core->mstatus &= ~(*value);
		break;
	}
}

static void csr_sie_read(struct r5sim_core *core,
			 struct r5sim_csr *csr)
{
	__raw_csr_write(&core->csr_file[CSR_SIE],
			core->mie & core->mideleg);
}

static void csr_sie_write(struct r5sim_core *core,
			  struct r5sim_csr *csr,
			  u32 type, u32 *value)
{
	u32 sie_mask = core->mideleg;
	u32 tmp_ie;

	*value &= sie_mask;

	switch (type) {
	case CSR_WRITE:
		tmp_ie = core->mie & ~sie_mask;
		core->mie = tmp_ie | *value;
		break;
	case CSR_SET:
		core->mie |= *value;
		break;
	case CSR_CLR:
		core->mie &= ~(*value);
		break;
	}
}

static void csr_sip_read(struct r5sim_core *core,
			 struct r5sim_csr *csr)
{
	__raw_csr_write(&core->csr_file[CSR_SIE],
			core->mip & core->mideleg);
}

static void csr_sip_write(struct r5sim_core *core,
			  struct r5sim_csr *csr,
			  u32 type, u32 *value)
{
	u32 sip_mask = core->mideleg;
	u32 tmp_ip;

	*value &= sip_mask;

	switch (type) {
	case CSR_WRITE:
		tmp_ip = core->mip & ~sip_mask;
		core->mip = tmp_ip | *value;
		break;
	case CSR_SET:
		core->mip |= *value;
		break;
	case CSR_CLR:
		core->mip &= ~(*value);
		break;
	}
}

/*
 * Timer callback so that the CSR timer function can get the host system
 * time. This generates a time in nanoseconds which is stored into the
 * TIME and TIMEH CSRs.
 *
 * The subsequent reads will then pull out this computed value from the
 * CSR file.
 */
static void r5sim_csr_time(struct r5sim_core *core,
			   struct r5sim_csr *csr)
{
	long nsecs;
	time_t secs;
	uint64_t delta_ns;
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	nsecs = now.tv_nsec - core->start.tv_nsec;
	secs = now.tv_sec - core->start.tv_sec;

	if (nsecs < 0) {
		secs = secs - 1;
		nsecs += 1000000000;
	}

	delta_ns = secs * 1000000000 + nsecs;

	__raw_csr_write(&core->csr_file[CSR_TIME], (u32)delta_ns);
	__raw_csr_write(&core->csr_file[CSR_TIMEH], (u32)(delta_ns >> 32));
}

void __r5sim_core_add_csr(struct r5sim_core *core,
			  struct r5sim_csr *csr_reg,
			  u32 csr)
{
	r5sim_assert(csr < 4096);

	core->csr_file[csr] = *csr_reg;
}

void r5sim_core_default_csrs(struct r5sim_core *core)
{
	r5sim_core_add_csr(core, CSR_CYCLE,	0x0, CSR_F_READ);
	r5sim_core_add_csr(core, CSR_INSTRET,	0x0, CSR_F_READ);
	r5sim_core_add_csr(core, CSR_CYCLEH,	0x0, CSR_F_READ);
	r5sim_core_add_csr(core, CSR_INSTRET,	0x0, CSR_F_READ);

	r5sim_core_add_csr_fn(core, CSR_TIME,	0x0, CSR_F_READ, r5sim_csr_time, NULL);
	r5sim_core_add_csr_fn(core, CSR_TIMEH,	0x0, CSR_F_READ, r5sim_csr_time, NULL);

	/*
	 * Machine mode CSRs.
	 */
	r5sim_core_add_csr(core, CSR_MISA,		0x40001100,	CSR_F_READ);
	r5sim_core_add_csr(core, CSR_MVENDORID,		0x0,		CSR_F_READ);
	r5sim_core_add_csr(core, CSR_MARCHID,		0x0,		CSR_F_READ);
	r5sim_core_add_csr(core, CSR_MIMPID,		0x0,		CSR_F_READ);
	r5sim_core_add_csr(core, CSR_MHARTID,		0x0,		CSR_F_READ);

	r5sim_core_add_csr_fn(core, CSR_MSTATUS,	0x0,		CSR_F_READ|CSR_F_WRITE, csr_mstatus_read, csr_mstatus_write);
	r5sim_core_add_csr_fn(core, CSR_MIE,		0x0,		CSR_F_READ|CSR_F_WRITE, csr_mie_read, csr_mie_write);
	r5sim_core_add_csr_fn(core, CSR_MIP,		0x0,		CSR_F_READ|CSR_F_WRITE, csr_mip_read, csr_mip_write);
	r5sim_core_add_csr_fn(core, CSR_MEDELEG,	0x0,		CSR_F_READ|CSR_F_WRITE, NULL, csr_medeleg_write);
	r5sim_core_add_csr_fn(core, CSR_MIDELEG,	0x0,		CSR_F_READ|CSR_F_WRITE, NULL, csr_mideleg_write);

	r5sim_core_add_csr(core, CSR_MTVEC,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MSCRATCH,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MEPC,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MCAUSE,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MTVAL,		0x0,		CSR_F_READ|CSR_F_WRITE);

	/*
	 * Supervisor CSRs.
	 */
	r5sim_core_add_csr_fn(core, CSR_SSTATUS,	0x0,		CSR_F_READ|CSR_F_WRITE, csr_sstatus_read, csr_sstatus_write);
	r5sim_core_add_csr_fn(core, CSR_SIE,		0x0,		CSR_F_READ|CSR_F_WRITE, csr_sie_read, csr_sie_write);
	r5sim_core_add_csr_fn(core, CSR_SIP,		0x0,		CSR_F_READ|CSR_F_WRITE, csr_sip_read, csr_sip_write);

	r5sim_core_add_csr(core, CSR_STVEC,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_SSCRATCH,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_SEPC,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_SCAUSE,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_STVAL,		0x0,		CSR_F_READ|CSR_F_WRITE);

	/*
	 * The PMP address registers.
	 */
	r5sim_core_add_csr_fn(core, CSR_PMPADDR0,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR1,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR2,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR3,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR4,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR5,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR6,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR7,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR8,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR9,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR10,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR11,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR12,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR13,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR14,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPADDR15,	0x0,		CSR_F_READ|CSR_F_WRITE, pmpaddr_rd, pmpaddr_wr);

	r5sim_core_add_csr_fn(core, CSR_PMPCFG0,	0x0,		CSR_F_READ|CSR_F_WRITE|CSR_F_SKIP_WRITE, pmpcfg_rd, pmpcfg_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPCFG1,	0x0,		CSR_F_READ|CSR_F_WRITE|CSR_F_SKIP_WRITE, pmpcfg_rd, pmpcfg_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPCFG2,	0x0,		CSR_F_READ|CSR_F_WRITE|CSR_F_SKIP_WRITE, pmpcfg_rd, pmpcfg_wr);
	r5sim_core_add_csr_fn(core, CSR_PMPCFG3,	0x0,		CSR_F_READ|CSR_F_WRITE|CSR_F_SKIP_WRITE, pmpcfg_rd, pmpcfg_wr);

	/* Custom CSRs. */
	r5sim_core_add_csr_fn(core, CSR_CUSTOM_SIMEXIT,	0x0,		CSR_F_READ|CSR_F_WRITE, NULL, csr_sim_exit);
}

/*
 * All CSR instuctions read the CSR into rd; it's really only after that
 * the instructions have different effects.
 *
 * This will return NULL if the CSR is not implemented. Otherwise it
 * returns the address of the CSR struct in the CSR file.
 */
struct r5sim_csr *__csr_always(struct r5sim_core *core, u32 rd, u32 csr)
{
	struct r5sim_csr *csr_reg;

	r5sim_assert(csr < 4096);

	csr_reg = &core->csr_file[csr];

	if ((csr_reg->flags & CSR_F_PRESENT) == 0)
		return NULL;

	/*
	 * Check the CSR for required privileges.
	 */
	if (get_field(csr, CSR_PRIV_FIELD) > core->priv)
		return NULL;

	if ((csr_reg->flags & CSR_F_READ) != 0) {
		if (csr_reg->read_fn)
			csr_reg->read_fn(core, csr_reg);
		__set_reg(core, rd, __raw_csr_read(csr_reg));
	}

	return csr_reg;
}

int __csr_w(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return -1;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg, CSR_WRITE, &value);
		__raw_csr_write(csr_reg, value);
		r5sim_dbg("CSR [W] %-15s v=0x%08x\n", csr_reg->name, value);
	}

	return 0;
}

int __csr_s(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return -1;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg, CSR_SET, &value);
		__raw_csr_set_mask(csr_reg, value);
		r5sim_dbg("CSR [S] %-15s v=0x%08x\n", csr_reg->name, value);
	}

	return 0;
}

int __csr_c(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return -1;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg, CSR_CLR, &value);
		__raw_csr_clear_mask(csr_reg, value);
		r5sim_dbg("CSR [C] %-15s v=0x%08x\n", csr_reg->name, value);
	}

	return 0;
}

u32 csr_read(struct r5sim_core *core, u32 csr)
{
	r5sim_assert(csr < 4096);

	return __raw_csr_read(&core->csr_file[csr]);
}

void csr_write(struct r5sim_core *core, u32 csr, u32 value)
{
	r5sim_assert(csr < 4096);

	return __raw_csr_write(&core->csr_file[csr], value);
}

u32 r5sim_csr_index(struct r5sim_core *core,
		    struct r5sim_csr *csr)
{
	return csr - core->csr_file;
}
