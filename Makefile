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
# Build the r5sim!
#

MAKEFLAGS += -rR

R5SIM_DIR   = $(PWD)
INSTALL_DIR = $(R5SIM_DIR)/bin
SCRIPTS     = $(R5SIM_DIR)/scripts

# Sub-directories to build.
SUBDIRS     = src brom conftest

VERBOSE     = @
ifeq ($(V),1)
VERBOSE     =
endif

CONFIG      =
ifneq ($(C),)
CONFIG      = $(realpath $(C))
endif

MAKE.R      = $(MAKE) -f $(R5SIM_DIR)/build/recurse.mk --no-print-directory

# Export some environment variables for the sub-makefiles.
export R5SIM_DIR INSTALL_DIR SCRIPTS VERBOSE CONFIG

.PHONY: install build $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE.R) -C $@ build
	@$(MAKE.R) -C $@ install

clean:
	@for d in $(SUBDIRS); do			\
		echo Cleaning: $$d;			\
		$(MAKE.R) -C $$d clean C=$(C) ;		\
	done

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)
