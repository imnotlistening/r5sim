#
# Build the r5sim!
#

R5SIM_DIR   = $(PWD)
INSTALL_DIR = $(R5SIM_DIR)/bin
SCRIPTS     = $(R5SIM_DIR)/scripts

# Sub-directories to build.
SUBDIRS     = src brom conftest

MAKEFILES   = $(R5SIM_DIR)/build/Makefile.defaults \
              $(R5SIM_DIR)/build/Makefile.rules

VERBOSE   = @
ifeq ($(V),1)
VERBOSE   =
endif

# Export some environment variables for the sub-makefiles.
export R5SIM_DIR INSTALL_DIR SCRIPTS MAKEFILES VERBOSE

.PHONY: install build

all: install

build:
	@for d in $(SUBDIRS); do				\
		echo Building: $$d;				\
		$(MAKE) --no-print-directory -C $$d build;	\
	done

install: build $(INSTALL_DIR)
	@for d in $(SUBDIRS); do				\
		echo Installing: $$d;				\
		$(MAKE) --no-print-directory -C $$d install;	\
	done

clean:
	@for d in $(SUBDIRS); do				\
		echo Cleaning: $$d;				\
		$(MAKE) --no-print-directory -C $$d clean;	\
	done

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)
