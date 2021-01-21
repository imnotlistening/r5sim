/*
 * This is the entrance for the "conformance" test for the simple-core R5.
 * It's probably not, nor likely ever, to be a really good conformance test
 * but it should hopefully prevent any crippling regressions from creeping
 * into the simple-core.
 */

.globl _start
.globl start

#define DRAM_START	0x20000000
#define DRAM_STACK	0x30000000

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

/*
 * Trap vector trampoline. This will be jumped to when a trap occurs.
 * From there we call the C mode trap handler trap_entrance().
 *
 * Once that trap handler returns we can increment the PC and move on
 * with our lives since we don't do any fancy page fault stuff. This
 * is just for testing.
 *
 * To actually call the C code, we'll need to save the original
 * context's execution environment. To do that we'll start by writing
 * all of the context's registers onto the stack. When we return we'll
 * pop them.
*/
.globl trap_entrance
.globl __trap_vector
__trap_vector:
	addi	sp,sp,-122
	sw	ra,  0(sp)
	sw	gp,  4(sp)
	sw	tp,  8(sp)
	sw	t0,  12(sp)
	sw	t1,  16(sp)
	sw	t2,  20(sp)
	sw	s0,  24(sp)
	sw	s1,  28(sp)
	sw	a0,  32(sp)
	sw	a1,  36(sp)
	sw	a2,  40(sp)
	sw	a3,  44(sp)
	sw	a4,  48(sp)
	sw	a5,  52(sp)
	sw	a6,  56(sp)
	sw	a7,  60(sp)
	sw	s2,  64(sp)
	sw	s3,  68(sp)
	sw	s4,  72(sp)
	sw	s5,  76(sp)
	sw	s6,  80(sp)
	sw	s7,  84(sp)
	sw	s8,  88(sp)
	sw	s9,  92(sp)
	sw	s10, 96(sp)
	sw	s11, 100(sp)
	sw	t3,  104(sp)
	sw	t4,  108(sp)
	sw	t5,  112(sp)
	sw	t6,  116(sp)

	csrr	a0,0x341	/* CSR_MEPC */
	csrr	a1,0x342	/* CSR_MCAUSE */
	jal	trap_entrance

	/*
	 * Increment the MEPC counter to the next instruction so
	 * we don't just immediately trap again.
	 */
	csrr	t0,0x341
	addi	t0,t0,4
	csrw	0x341,t0

	lw	ra,  0(sp)
	lw	gp,  4(sp)
	lw	tp,  8(sp)
	lw	t0,  12(sp)
	lw	t1,  16(sp)
	lw	t2,  20(sp)
	lw	s0,  24(sp)
	lw	s1,  28(sp)
	lw	a0,  32(sp)
	lw	a1,  36(sp)
	lw	a2,  40(sp)
	lw	a3,  44(sp)
	lw	a4,  48(sp)
	lw	a5,  52(sp)
	lw	a6,  56(sp)
	lw	a7,  60(sp)
	lw	s2,  64(sp)
	lw	s3,  68(sp)
	lw	s4,  72(sp)
	lw	s5,  76(sp)
	lw	s6,  80(sp)
	lw	s7,  84(sp)
	lw	s8,  88(sp)
	lw	s9,  92(sp)
	lw	s10, 96(sp)
	lw	s11, 100(sp)
	lw	t3,  104(sp)
	lw	t4,  108(sp)
	lw	t5,  112(sp)
	lw	t6,  116(sp)

	addi	sp,sp,122
	mret
