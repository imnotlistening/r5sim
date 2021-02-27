/*
 * Machine description and loader.
 */

#ifndef __R5SIM_MACHINE__
#define __R5SIM_MACHINE__

#include <pthread.h>

#include <r5sim/env.h>
#include <r5sim/list.h>

struct r5sim_core;

/*
 * Define a limited number of breakpoints. _each_ instruction has to check
 * against this list, so it's expensive to have too many.
 */
#define BREAKPOINT_NR		4

#define MACH_ACCESS_OK		 0
#define MACH_ACCESS_MISALIGN	-1
#define MACH_ACCESS_FAULT	-2

/*
 * Define a "machine". This is a single core - for now - and some memory.
 * Define several function pointers for accessing memory, device memory,
 * etc.
 */
struct r5sim_machine {
	struct {
		const char *name;
	} descr;

	/*
	 * Each core should keep track of this field. If it flips to true then
	 * they should stop executing and return. Execution will be started again
	 * once the debug session exits.
	 *
	 * The main execution thread should set the signal_debug variable back to
	 * false. When the debugger detects this, it'll know that the machine now
	 * belongs to it.
	 *
	 * Once the debug session is done, the debugger should signal debug_done
	 * so that the main thread can wake up.
	 */
	volatile int       debug;
	volatile int       step;

	struct r5sim_core *core;

	/*
	 * Base memory address and size; this is "DRAM".
	 */
	u32    memory_base;
	u32    memory_size;
	u8    *memory;

	u32    brom_base;
	u32    brom_size;
	u8    *brom;

	u32    iomem_base;
	u32    iomem_size;

	/*
	 * HW breakpoints. Each core checks these when loading an instruction
	 * to determine if it should break.
	 *
	 * If any break points are set, then breaks_set is 1. This provies a
	 * short circuit for the general case where no breakpoints are set.
	 */
	u32    hwbreaks[BREAKPOINT_NR];
	int    breaks_set;

	/*
	 * Load and store - will direct the loads and stores to either DRAM
	 * or the relevant IO device.
	 *
	 * Return 0 if the load/store was successful. If unsuccessful it
	 * can return:
	 *
	 *   - ML_ALIGN_FAULT
	 *   - ML_PAGE_FAULT
	 *   - ML_ACCESS_FAULT
	 *
	 * Note that only the first is currently supported since there's no
	 * MMU yet.
	 */
	int       (*memload32)(struct r5sim_machine *mach,
			       u32 paddr,
			       u32 *dest);
	int       (*memload16)(struct r5sim_machine *mach,
			       u32 paddr,
			       u16 *dest);
	int       (*memload8)(struct r5sim_machine *mach,
			      u32 paddr,
			      u8 *dest);

	int       (*memstore32)(struct r5sim_machine *mach,
				u32 paddr,
				u32 value);
	int       (*memstore16)(struct r5sim_machine *mach,
				u32 paddr,
				u16 value);
	int       (*memstore8)(struct r5sim_machine *mach,
			       u32 paddr,
			       u8 value);

	/*
	 * List of IO devices, e.g UARTs.
	 */
	struct list_head io_devs;
};

/*
 * Load the default machine; this is a machine that can be used if no
 * other machine is specified and loaded.
 *
 * TODO: Dynamic machine loading.
 */
struct r5sim_machine *r5sim_machine_load_default(void);

/*
 * Load a bootrom image into the brom space in the machine. If this
 * fails it triggers an assert.
 */
void r5sim_machine_load_brom(struct r5sim_machine *mach);

/*
 * Begin machine boot.
 */
void r5sim_machine_run(struct r5sim_machine *mach);

/*
 * Display the details for a machine.
 */
void r5sim_machine_print(struct r5sim_machine *mach);

#endif
