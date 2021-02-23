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

MAKE.R      = $(MAKE) -f $(R5SIM_DIR)/build/recurse.mk --no-print-directory

# Export some environment variables for the sub-makefiles.
export R5SIM_DIR INSTALL_DIR SCRIPTS VERBOSE

.PHONY: install build $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE.R) -C $@ build
	@$(MAKE.R) -C $@ install

clean:
	@for d in $(SUBDIRS); do			\
		echo Cleaning: $$d;			\
		$(MAKE.R) -C $$d clean ;		\
	done

$(INSTALL_DIR):
	@mkdir -p $(INSTALL_DIR)
