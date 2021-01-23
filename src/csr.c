/*
 * Basic CPU core interfaces.
 */

#include <stdlib.h>

#include <time.h>

#include <r5sim/env.h>
#include <r5sim/core.h>

static void csr_mstatus_read(struct r5sim_core *core,
		 struct r5sim_csr *csr)
{

}

static void csr_mstatus_write(struct r5sim_core *core,
		  struct r5sim_csr *csr)
{

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

static void r5sim_core_default_csrs(struct r5sim_core *core)
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
	r5sim_core_add_csr(core, CSR_MEDELEG,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MIDELEG,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MIE,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MTVEC,		0x0,		CSR_F_READ|CSR_F_WRITE);

	r5sim_core_add_csr(core, CSR_MSCRATCH,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MEPC,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MCAUSE,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MTVAL,		0x0,		CSR_F_READ|CSR_F_WRITE);
	r5sim_core_add_csr(core, CSR_MIP,		0x0,		CSR_F_READ|CSR_F_WRITE);


}

void r5sim_core_init_common(struct r5sim_core *core)
{
	r5sim_core_default_csrs(core);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &core->start);
}

/*
 * All CSR instuctions read the CSR into rd; it's really only after that
 * the instructions have different effects.
 *
 * This will return NULL if the CSR is not implemented. Otherwise it
 * returns the address of ther CSR struct in the CSR file.
 */
static struct r5sim_csr *__csr_always(struct r5sim_core *core,
				      u32 rd, u32 csr)
{
	struct r5sim_csr *csr_reg;

	r5sim_assert(csr < 4096);

	csr_reg = &core->csr_file[csr];

	if ((csr_reg->flags & CSR_F_PRESENT) == 0)
		return NULL;

	if ((csr_reg->flags & CSR_F_READ) != 0 && rd != 0) {
		if (csr_reg->read_fn)
			csr_reg->read_fn(core, csr_reg);
		__set_reg(core, rd, __raw_csr_read(csr_reg));
	}

	return csr_reg;
}

void __csr_w(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_write(csr_reg, value);
	}
}

void __csr_s(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_set_mask(csr_reg, value);
	}
}

void __csr_c(struct r5sim_core *core, u32 rd, u32 value, u32 csr)
{
	struct r5sim_csr *csr_reg;

	csr_reg = __csr_always(core, rd, csr);
	if (csr_reg == NULL)
		return;

	if ((csr_reg->flags & CSR_F_WRITE) != 0) {
		if (csr_reg->write_fn)
			csr_reg->write_fn(core, csr_reg);
		__raw_csr_clear_mask(csr_reg, value);
	}
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
