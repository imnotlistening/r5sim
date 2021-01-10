/*
 * Application stuff; e.g the command line args to the simulator, etc.
 * Useful for code that wants to look at these things.
 */

#ifndef __R5SIM_APP_H__
#define __R5SIM_APP_H__

struct r5sim_app_args {
	int         help;
	int         verbose;
	int         itrace;
	const char *bootrom;
	const char *disk_file;
};

struct r5sim_app_args *
r5sim_app_get_args(void);

#endif
