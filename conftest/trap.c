/*
 * Trap handler and some trap setup.
 */

#include "csr.h"
#include "conftest.h"

static const char *
code_to_str(uint32_t code)
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

int
trap_setup(void)
{
	uint32_t mtvec;
	uint32_t tvec = (uint32_t)&__trap_vector;

	tvec <<= 2;

	write_csr(0x305, tvec);
	read_csr(0x305, mtvec);

	printf("mtvec = 0x%08x\n", mtvec);

	return 0;
}

int
trap_test(void)
{
	printf("Issuing illegal instruction.\n");
	asm volatile(".long 0x00000000\n\t");
	printf("We have returned!\n");

	printf("Issuing ECALL.\n");
	asm volatile("ecall\n\t");
	printf("We have returned!\n");

	return 1;
}

void
trap_entrance(uint32_t trap_pc, uint32_t code)
{
	printf(" -- Received a trap: %u (%s)!\n", code, code_to_str(code));
	printf(" -- Old PC: 0x%08x!\n", trap_pc);
}
