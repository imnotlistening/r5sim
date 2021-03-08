# Copyright 2021, Alex Waterman <imnotlistening@gmail.com> -*- mode: makefile -*-
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
# RISC-V specifics for building risc-v targeted binaries.
#

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc
LD        = riscv64-linux-gnu-ld
OBJCOPY   = riscv64-linux-gnu-objcopy
NM        = riscv64-linux-gnu-nm

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32im -mabi=ilp32 -O2 -fno-omit-frame-pointer
LDFLAGS   = -melf32lriscv -nostartfiles -nostdlib -nodefaultlibs
