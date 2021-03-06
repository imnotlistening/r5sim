#!/usr/bin/python3
#
# Copyright 2021, Alex Waterman <imnotlistening@gmail.com>
#
# This file is part of r5sim.
#
# r5sim is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# r5sim is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with r5sim.  If not, see <https://www.gnu.org/licenses/>.
# Some general rules that can be used for building C files, etc.
#
# Take our input, in the form of NM's output, and generate a C file
# that turns this into an array of address -> name pairs.
#

import sys

entries = 0

# Some prologue for the symbol table.
print(
    """/*
 * This file is AUTOGENERATED! Do not edit by hand. This is generated
 * by syms.py.
 */

.section	.rodata.symtable
""")

for line in sys.stdin:
    __addr, symtype, name = line.split()

    addr = int(__addr, 16)

    print("\t.word  0x%x" % addr)
    print("\t.ascii \"%s\"" % name[0:59])
    print("\t.balign 64")

    entries += 1

# Now print the table header info.
print("")
print(".section	.rodata.symtable.header")
print("\t.word  %d /* length */" % entries)
