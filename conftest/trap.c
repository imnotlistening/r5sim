/*
 * Trap handler and some trap setup.
 */

#include "csr.h"
#include "conftest.h"

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

	tvec <<= 2;

	write_csr(0x305, tvec);
	read_csr(0x305, mtvec);

	printf("mtvec = 0x%08x\n", mtvec);

	return 0;
}

void trap_entrance(u32 trap_pc, u32 code)
{
	/*
	 * Do nothing!
	 */
}
