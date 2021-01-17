/*
 * This is the entrance for the "conformance" test for the simple-core R5.
 * It's probably not, nor likely ever, to be a really good conformance test
 * but it should hopefully prevent any crippling regressions from creeping
 * into the simple-core.
 */

.globl _start
.globl start

#define DRAM_START	0x40000000
#define DRAM_STACK	0x50000000

/*
 * The bootrom loads 16Kb into the very bottom of DRAM from the
 * start of the disk.
 *
 * This section of code - .text.boot - is explicitly placed at offset 0x0
 * in the in the output binary and will be the first code to execute.
 *
 * This sets up the basic C environment that the subsequent C code needs
 * and jumps to start(), defined in start.c. If start() returns then we
 * sit in an infinite loop waiting on the UART.
 */
.section	.text.boot
_start:
	/*
	 * Load a stack pointer for the C code to use.
	 */
	lui	sp, %hi(DRAM_STACK)

	/*
	 * Jump to start!
	 */
	jal	start

	/*
	 * Just sit in an infinite loop reading from the UART.
	 */
	lui	t0,%hi(0x4000000)
.loop:
	lw	t1, 0(t0)
	j	.loop

.section	.text

/*
 * uint32_t ct_rdcycle(void)
 *
 * Reads and returns the cycle counter from the CYCLE CSR.
 */
.globl ct_rdcycle
ct_rdcycle:
	rdcycle		a0
	ret

/*
 * uint32_t ct_rdinstret(void)
 *
 * Reads and returns the instret (instructions retired - i.e number of
 * instructions fully executed) counter from the CYCLE CSR. instret and
 * cycles both increment by 1 for each instruction on the simle core.
 * There's no pipelining or anything like that.
 */
.globl ct_rdinstret
ct_rdinstret:
	rdinstret	a0
	ret

/*
 * void ct_rdtime(struct ct_time *time)
 *
 * Reads the core time stamp counter and places the results into *lo and
 * *hi.
 */
.globl ct_rdtime
ct_rdtime:
	/*
	 * Read the time; try again if necessary.
	 */
	rdtimeh		t1
	rdtime		t0
	rdtimeh		t2
	bne		t1, t2, ct_rdtime

	/*
	 * Store the values per lo and hi.
	 */
	sw		t0, 0(a0)
	sw		t1, 4(a0)

	ret
