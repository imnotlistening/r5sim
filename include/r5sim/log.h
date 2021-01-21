/*
 * Logging routines for printing simulation info to the terminal.
 */

#ifndef __R5SIM_LOG_H__
#define __R5SIM_LOG_H__

enum r5sim_log_level {
	ERR = 0,
	WARN,
	INFO,
	DEBUG,
	DEBUG_V,
	DEBUG_VV,
};

#define r5sim_err(fmt, args...)						\
	__r5sim_log_print(ERR, __FILE__, __LINE__, fmt, ##args)

#define r5sim_warn(fmt, args...)					\
	__r5sim_log_print(WARN, __FILE__, __LINE__, fmt, ##args)

#define r5sim_info(fmt, args...)					\
	__r5sim_log_print(INFO, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg(fmt, args...)						\
	__r5sim_log_print(DEBUG, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg_v(fmt, args...)					\
	__r5sim_log_print(DEBUG_V, __FILE__, __LINE__, fmt, ##args)

#define r5sim_dbg_vv(fmt, args...)					\
	__r5sim_log_print(DEBUG_VV, __FILE__, __LINE__, fmt, ##args)

__attribute__((format (printf, 4, 5)))
void
__r5sim_log_print(enum r5sim_log_level lvl,
		  const char *file,
		  int line,
		  const char *fmt, ...);

/*
 * Separate from the rest of the logging - itrac'ing is _very_ verbose.
 */
#define r5sim_itrace(fmt, args...)					\
	__r5sim_itrace_print(fmt, ##args)

__attribute__((format (printf, 1, 2)))
void
__r5sim_itrace_print(const char *fmt, ...);

#endif
