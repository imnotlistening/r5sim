/*
 * Define a few functions to help out with keeping track of
 * time and computing core perf.
 */

#ifndef __CT_TIME_H__
#define __CT_TIME_H__

#include <ct/types.h>

struct ct_time {
	u32 lo;
	u32 hi;
};

u32  ct_compute_perf(void);
void ct_time_diff(struct ct_time *dst, struct ct_time *a, struct ct_time *b);
void ct_ptime(struct ct_time *t, const char *str);

#endif
