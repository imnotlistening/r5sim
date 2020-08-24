/*
 * A simple core to do simple simulation!
 */

#ifndef __R5SIM_SIMPLE_CORE_H__
#define __R5SIM_SIMPLE_CORE_H__

struct r5sim_core;
struct r5sim_machine;

struct r5sim_core *
r5sim_simple_core_instance(struct r5sim_machine *mach);

#endif
