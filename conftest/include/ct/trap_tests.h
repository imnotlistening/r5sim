/*
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
