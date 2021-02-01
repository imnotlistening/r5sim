#
# Build the r5sim!
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

# Default host CC and some basic CFLAGS.
CC        = gcc
LD        = ld
CFLAGS    = -Wall -Werror

R5SIM_DIR = $(PWD)

# Install directory.
INSTALL   = $(R5SIM_DIR)/bin

# Sub-directories to build.
SUBDIRS   = src brom conftest

# Export some environment variables for the sub-makefiles.
export CC CFLAGS R5SIM_DIR INSTALL

.PHONY: $(SUBDIRS) all clean install

all: $(SUBDIRS)

$(INSTALL):
	@mkdir -p $(INSTALL)

$(SUBDIRS):  | $(INSTALL)
	@echo "Building: $@"
	@+$(MAKE) --no-print-directory -C $@

	@echo "Installing: $@"
	@+$(MAKE) --no-print-directory -C $@ install

clean:
	@for d in $(SUBDIRS); do				\
		echo Cleaning: $$d;				\
		$(MAKE) --no-print-directory -C $$d clean;	\
	done
