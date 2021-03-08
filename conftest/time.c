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
 * Some time and timing functions.
 */

#include <ct/time.h>
#include <ct/conftest.h>

/*
 * Compute a - b and store the result in dst.
 */
void ct_time_diff(struct ct_time *dst,
		  struct ct_time *a,
		  struct ct_time *b)
{
	dst->lo = a->lo - b->lo;
	dst->hi = a->hi - b->hi;

	if (a->lo < b->lo) {
		dst->hi -= 1;
		dst->lo = 0xffffffff - b->lo + a->lo + 1;
	}
}

void ct_ptime(struct ct_time *t, const char *str)
{
	printf("%s {%u.%u}\n", str, t->hi, t->lo);
}

u32 ct_compute_perf(void)
{
	struct ct_time start, end, diff;
	u32 cycles_start = 0, cycles_end = 0;
	volatile u32 a, b, c, d;
	u32 i, iterations = 2000000;

	a = 1;
	b = 2;
	c = 3;
	d = 4;

	ct_rdtime(&start);
	cycles_start = ct_rdcycle();

	/*
	 * Run a mix of arithmetic instructions. Get a rough idea for
	 * the CPU "perf".
	 */
	for (i = 0; i < iterations; i++) {
		a += b;
		b *= c;
		c -= d;
	}

	cycles_end = ct_rdcycle();
	ct_rdtime(&end);
	ct_time_diff(&diff, &end, &start);

	printf("Perf test:\n");
	printf("  Cycles:          %u\n", cycles_end - cycles_start);
	printf("  Time taken (ns): %u:%u\n", diff.hi, diff.lo);

	/*
	 * Quick and dirty calculation for how many MIPS we are executing.
	 * This assumes that diff.hi is 0 - e.g this takes less than 4 ish
	 * seconds to execute (what type of programmer are you?! A lazy
	 * one). Accuracy is in the ms range; that is we round our ns
	 * counter to ms so that when we divide by time, we don't just wind
	 * up with 0.
	 */
	printf("  IPS:             %u\n",
	       ((cycles_end - cycles_start) /
		(diff.lo / 1000000)) * 1000);

	return d;
}
