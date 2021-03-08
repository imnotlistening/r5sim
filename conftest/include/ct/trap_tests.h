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
 * Some common defs for all trap testing.
 */

#ifndef __CT_TRAP_TESTS_H__
#define __CT_TRAP_TESTS_H__

struct ct_excep_test {
	void (*fault_func)(void);
};

extern struct ct_excep_test ld_align;
extern struct ct_excep_test st_align;
extern struct ct_excep_test inst_align;
extern struct ct_excep_test ld_fault;
extern struct ct_excep_test st_fault;
extern struct ct_excep_test illegal_inst;
extern struct ct_excep_test ecall;

int ct_test_exception(void *__test);
int ct_test_sw_intr(void *data);
int ct_test_timer_intr(void *data);

#endif
