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
 * Trap handler and some trap setup.
 */

#include <ct/csr.h>
#include <ct/conftest.h>

typedef void (*intr_handler)(u32 trap_pc, u32 code);

volatile u32 delegate_smode = 0;
volatile u32 m_intr_exec = 0;
volatile u32 s_intr_exec = 0;
volatile u32 m_excep_exec = 0;
volatile u32 s_excep_exec = 0;
volatile u32 expect_exception = 0;

__attribute__((unused))
static const char *code_to_str(u32 code)
{
	switch (code) {
	case TRAP_ILLEGAL_INST:
		return "ILLEGAL_INSTRUCTION";
	case TRAP_ECALL_MMODE:
		return "ECALL (MMode)";
	default:
		return "Unknown!";
	}
}

int trap_setup(void)
{
	u32 mtvec;
	u32 tvec = (u32)&__trap_vector;
	u32 mstatus, mie;

	tvec <<= 2;

	write_csr(CSR_MTVEC, tvec);
	read_csr(CSR_MTVEC, mtvec);

	/*
	 * Enable interrupts and unmask the MSW interrupt.
	 */
	read_csr(CSR_MSTATUS, mstatus);
	read_csr(CSR_MIE,     mie);

	set_field(mstatus, CSR_MSTATUS_MIE, 1);

	set_field(mie,     CSR_MIE_MSIE,    1);
	set_field(mie,     CSR_MIE_MTIE,    1);

	write_csr(CSR_MSTATUS, mstatus);
	write_csr(CSR_MIE,     mie);

	return 0;
}

void intr_msi(u32 trap_pc, u32 code)
{
	/* Just clear the interrupt. */
	clear_csr(CSR_MIP, 1 << code);
	m_intr_exec = 1;
}

void intr_timer(u32 trap_pc, u32 code)
{
	/*
	 * Just clear the interrupt. Technically in violation of the
	 * ISA w.r.t. timers. We'll need to fix this at some point.
	 */
	clear_csr(CSR_MIP, 1 << code);
	m_intr_exec = 1;
}

void intr_msi_sv(u32 trap_pc, u32 code)
{
	/* Just clear the interrupt. */
	clear_csr(CSR_SIP, 1 << code);
	s_intr_exec = 1;
}

void intr_timer_sv(u32 trap_pc, u32 code)
{
	/*
	 * Just clear the interrupt. Technically in violation of the
	 * ISA w.r.t. timers. We'll need to fix this at some point.
	 */
	clear_csr(CSR_SIP, 1 << code);
	s_intr_exec = 1;
}

void __unknown_intr(u32 trap_pc, u32 code)
{
	printf("\n");
	printf("--- !! ---!\n");
	printf("Unknown interrupt:\n");
	printf("  Source PC: 0x%08x\n", trap_pc);
	printf("  Code:      %u\n", code);

	backtrace_addr(trap_pc, 0);

	while (1)
		;
}

void trap_entrance(u32 trap_pc, u32 code, u32 trap_fp)
{
	/*
	 * ECall from S-Mode. Delegate all exceptions to S-Mode.
	 */
	if (code == 0x9 && delegate_smode) {
		write_csr(CSR_MEDELEG, 0xffff);
		write_csr(CSR_MIDELEG, 0xaa);
	}

	m_excep_exec = 1;

	if (expect_exception) {
		expect_exception = 0;
		return;
	}

	printf("\n");
	printf("--- !! ---\n");
	printf("Exception @ 0x%08x\n", trap_pc);
	printf("  Code: %u\n", code);

	backtrace_addr(trap_pc, trap_fp);

	while (1)
		;
}

void trap_entrance_sv(u32 trap_pc, u32 code, u32 trap_fp)
{
	s_excep_exec = 1;

	if (expect_exception) {
		expect_exception = 0;
		return;
	}

	printf("\n");
	printf("--- !! ---\n");
	printf("Exception @ 0x%08x\n", trap_pc);
	printf("  Code: %u\n", code);

	backtrace_addr(trap_pc, trap_fp);

	while (1)
		;
}

intr_handler intr_handlers[16] = {
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	intr_msi,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	intr_timer,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
};

intr_handler intr_handlers_sv[16] = {
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	intr_msi_sv,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	intr_timer_sv,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,

	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
	__unknown_intr,
};
