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
 * Provide access to an autogenerated symbol table.
 */

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include "types.h"

struct symbol {
	u32         addr;
	const char  sym[60];
};

struct symtable_header {
	u32 length;
};

const char *addr2sym(u32 addr, u32 *offset);
void        symtable_describe(void);

#endif
