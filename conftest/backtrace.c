/*
 * An attempt at building some backtracing code.
 */

#include "conftest.h"
#include "symbols.h"

#define stack_pointer(sp)			\
	asm volatile ("mv	%0, sp\n\t"	\
		      : "=r" (*sp) : )
#define return_address(ra)			\
	asm volatile ("mv	%0, ra\n\t"	\
		      : "=r" (*ra) : )
#define frame_pointer(fp)			\
	*(fp) = (u32)__builtin_frame_address(0)

struct stackframe {
	u32 fp;
	u32 ra;
};

/*
 * Print a backtrace for the calling function.
 */
void backtrace(void)
{
	u32 fp, sp, ra, pc;
	u32 i = 0;
	struct stackframe *frame;

	stack_pointer(&sp);
	frame_pointer(&fp);
	return_address(&ra);

	while (1) {
		/*
		 * If the frame pointer is 0, then we've passed the
		 * end of the stack.
		 */
		if (fp == 0u) {
			printf("End of trace (0x%x)!\n", fp);
			break;
		}

		frame = (struct stackframe *)fp - 1;
		sp = fp;
		fp = frame->fp;
		pc = frame->ra;

		printf("Frame %2u: 0x%08x %s()\n", i++, pc, addr2sym(pc));
	}
}

int btt_function3(void)
{
	int f = 3;
	u32 sp;
	const char *func = __func__;

	printf("  * Function: %u (%s)\n", (u32)f, func);

	backtrace();

	stack_pointer(&sp);

	f += (int)(sp & 0xf);

	return f;
}

int btt_function2(void)
{
	int f = 2;

	printf("  * Function: %u (%s)\n", (u32)f, __func__);
	f += btt_function3();

	return f;
}

int btt_function1(void)
{
	int f = 1;

	printf("  * Function: %u (%s)\n", (u32)f, __func__);
	f += btt_function2();

	return f * 4;
}

int backtrace_test(void)
{
	int x = 0;
	int y = 10;
	int z = -3;
	int q;

	int *y_p = &y;

	q = *y_p * z + x;

	printf("-- Backtrace test! --\n");

	q += btt_function1();

	printf("-- Backtrace test done! --\n");

	return q;
}
