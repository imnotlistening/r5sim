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
# Invoke this makefile to do a recursive make invocation in the requested
# directory. The expected usage is:
#
#   $(MAKE) -C <subdir> -f $(R5SIM_DIR)/build/recurse.mk <target>
#
# This will automatically include the necessary makefiles to pull in rules
# and other env stuff.
#

include $(R5SIM_DIR)/build/Makefile.defaults

# Include the actual Makefile. This'll define OBJS, etc.
-include Makefile

# And now include the rules. We want these to come in _after_ the objs,
# etc, are defined sicne this makes defining the targets a lot easier.
include $(R5SIM_DIR)/build/Makefile.rules
