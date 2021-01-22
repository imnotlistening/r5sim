/*
 * A super simple bootrom program for the default R5 core.
 *
 * The execution environment here is very limited. There is no writeable
 * stack or DRAM yet.
 */

.globl _start
.globl start

#define DRAM_START	0x20000000
#define DRAM_STACK	0x30000000

/*
 * The truly first code; this is loaded at BROM_BASE + 0x0. This is the
 * CPU boot vector.
 *
 * Subsequent sections can be placed anywhere (within the BROM). Init enough
 * to call a special C routine.
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
