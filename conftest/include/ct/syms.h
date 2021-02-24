/*
 * During compiling, write the symbols in the first pass of linking
 * into a generated C file. This file relies on the following header
 * for defs.
 */

#ifndef __SYMS_H__
#define __SYMS_H__

#include "types.h"

struct symbol {
	u32         addr;
	const char *name;
};

const char *addr_to_sym(u32 addr);

#endif
