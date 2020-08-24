#
# Build various test programs, etc, for the R5.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32i -mabi=ilp32 -nostartfiles -nodefaultlibs

INCLUDES  =
LIBS      =

BINS      = r5-test
OBJS      =

.PHONY: all clean install

all: $(BINS) $(TESTS)
	@echo > /dev/null

install:
	@cp $(BINS) $(INSTALL)

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
