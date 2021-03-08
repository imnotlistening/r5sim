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
 * An attempt at building some backtracing code.
 */

#include <ct/conftest.h>
#include <ct/symbols.h>

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

static inline void __backtrace(u32 fp)
{
	u32 pc;
	u32 offset;
	struct stackframe *frame;
	const char *symbol;

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
		fp = frame->fp;
		pc = frame->ra;

		symbol = addr2sym(pc, &offset);

		printf("Frame: 0x%08x %s()+0x%x\n", pc, symbol, offset);
	}

}

void backtrace_addr(u32 pc, u32 fp)
{
	const char *symbol;
	u32 offset;

	/*
	 * This could be cleaner, but print the current frame first then
	 * call __backtrace() and get the rest of the frames.
	 */
	symbol = addr2sym(pc, &offset);
	printf("Frame: 0x%08x %s()+0x%x\n", pc, symbol, offset);

	__backtrace(fp);
}

/*
 * Print a backtrace for the calling function.
 */
void backtrace(void)
{
	u32 fp;

	frame_pointer(&fp);
	__backtrace(fp);
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
