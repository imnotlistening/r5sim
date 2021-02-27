/*
 * RISC-5 traps and trap handling.
 */

#ifndef __R5SIM_TRAP_H__
#define __R5SIM_TRAP_H__

#define TRAP_INST_ADDR_MISALIGN		0  /* Instruction address misaligned */
#define TRAP_INST_ACCESS_FAULT		1  /* Instruction access fault */
#define TRAP_INST_PAGE_FAULT		12 /* Instruction page fault */

#define TRAP_LD_ADDR_MISALIGN		4  /* Load address misaligned */
#define TRAP_LD_ACCESS_FAULT		5  /* Load access fault */
#define TRAP_LD_PAGE_FAULT		13 /* Load page fault */

#define TRAP_ST_ADDR_MISALIGN		6  /* Store/AMO address misaligned */
#define TRAP_ST_ACCESS_FAULT		7  /* Store/AMO access fault */
#define TRAP_ST_PAGE_FAULT		15 /* Store/AMO page fault */

#define TRAP_ECALL_UMODE		8  /* Environment call from U-mode */
#define TRAP_ECALL_SMODE		9  /* Environment call from S-mode */
#define TRAP_ECALL_MMODE		11 /* Environment call from M-mode */

#define TRAP_ILLEGAL_INST		2  /* Illegal instruction */
#define TRAP_BREAK_POINT		3  /* Breakpoint */

#define TRAP_RESERVED_10		10 /* Reserved */
#define TRAP_RESERVED_14		14 /* Reserved for future standard use */

/*
 * Since [0, 15] are real exceptions, we need something else to convey
 * a non-exception. We'll use negative numbers for that. This includes
 * the xRET instructions since they need to convey status to the
 * controlling exec loop.
 */
#define TRAP_ALL_GOOD			-1
#define TRAP_MRET			-2
#define TRAP_SRET			-3

#endif
