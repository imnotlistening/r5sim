/*
 * Trap handler and some trap setup.
 */

#include "csr.h"
#include "conftest.h"

typedef void (*intr_handler)(u32 trap_pc, u32 code);

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

	printf("MTVEC:   0x%08x\n", mtvec);

	/*
	 * Enable interrupts and unmask the MSW interrupt.
	 */
	read_csr(CSR_MSTATUS, mstatus);
	read_csr(CSR_MIE,     mie);

	set_field(mstatus, CSR_MSTATUS_MIE, 1);
	set_field(mie,     CSR_MIE_MSIE,    1);

	write_csr(CSR_MSTATUS, mstatus);
	write_csr(CSR_MIE,     mie);

	read_csr(CSR_MSTATUS, mstatus);
	read_csr(CSR_MIE,     mie);
	printf("MSTATUS: 0x%08x\n", mstatus);
	printf("MIE:     0x%08x\n", mie);

	return 0;
}

void trap_entrance(u32 trap_pc, u32 code)
{
	/*
	printf("Hi from a trap!");
	printf("  Trap PC:     0x%08x\n", trap_pc);
	printf("  Trap return: 0x%08x\n", trap_pc + 4);
	printf("  Code:        %s\n", code_to_str(code));
	*/
}

static void intr_msi(u32 trap_pc, u32 code)
{
	/*
	printf("Machine SW interrupt!\n");
	printf("  Source PC: 0x%08x\n", trap_pc);
	printf("  Code:      %u\n", code);
	*/

	/* Just clear the interrupt. */
	write_csr(CSR_MIP, 1 << code);
}

static void intr_timer(u32 trap_pc, u32 code)
{
	/* Just clear the interrupt. */
	write_csr(CSR_MIP, 1 << code);
}

static void __unknown_intr(u32 trap_pc, u32 code)
{
	printf("Unknown interrupt:\n");
	printf("  Source PC: 0x%08x\n", trap_pc);
	printf("  Code:      %u\n", code);

	/* Just clear the interrupt. */
	write_csr(CSR_MIP, 1 << code);
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
