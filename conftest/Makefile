#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc
LD        = riscv64-linux-gnu-ld

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32im -mabi=ilp32 -O2
LDFLAGS   = -melf32lriscv -nostartfiles -nostdlib -nodefaultlibs -T conftest.lds

INCLUDES  = -I$(R5SIM_DIR)/include

OBJS      = conftest.o \
            start.o \
            print.o \
            trap.o \
            sys.o \
            test_env.o \
	    test_system.o \
	    test_ldst.o \
	    test_muldiv.o \
	    test_op.o \
	    test_traps.o

.PHONY: all clean install

all: conftest.bin
	@echo > /dev/null

install:
	@riscv64-linux-gnu-objcopy -O binary conftest.bin $(INSTALL)/conftest

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
