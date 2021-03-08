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
 * Implement a small conformance test to see if our R5 simple core is
 * mostly functional.
 *
 * Each test sub-module should provide a list of tests to execute.
 * The test code will run through them and make sure they pass.
 * Make sure the list is NULL terminated.
 */

#ifndef __CONFTEST_H__
#define __CONFTEST_H__

#include "types.h"

struct ct_time;

/*
 * This is the S-Mode test's "ABI" to ask M-Mode to delegate traps
 * to S-Mode. S-Mode will do an ECall with this variable set to true,
 * and when that happens, M-Mode will delegate all exceptions to
 * S-Mode.
 */
extern volatile u32 delegate_smode;
extern volatile u32 m_intr_exec;
extern volatile u32 s_intr_exec;
extern volatile u32 m_excep_exec;
extern volatile u32 s_excep_exec;
extern volatile u32 expect_exception;

/*
 * No need for CPU barriers on the simple_core. But we do need to limit
 * compiler optimization.
 */
#define readl(addr)		*((volatile u32 *)(addr))
#define writel(addr, val)	*((volatile u32 *)(addr)) = (u32)(val)

/* Compiler barrier. */
#define barrier()		asm volatile("": : :"memory")

/*
 * Print some basic sys info.
 */
void ct_sys_info(void);

__attribute__((format (printf, 1, 2)))
int printf(const char *fmt, ...);

void start(void);
void sv_start(void);
void _smode_start(void);

int  trap_setup(void);
void __trap_vector(void);
void __trap_vector_sv(void);

void trap_entrance(u32 trap_pc, u32 cause, u32 trap_fp);
void trap_entrance_sv(u32 trap_pc, u32 cause, u32 trap_fp);

void backtrace(void);
void backtrace_addr(u32 pc, u32 fp);
int  backtrace_test(void);

/*
 * Defined in entry.S
 */
u32  ct_rdcycle(void);
u32  ct_rdinstret(void);
void ct_rdtime(struct ct_time *time);

#define TRAP_ILLEGAL_INST	2
#define TRAP_ECALL_MMODE	11

#endif
