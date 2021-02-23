#
# Build a bootrom binary for the default r5 core.
#

include $(R5SIM_DIR)/build/riscv-xc.mk

INCLUDES  = -I$(R5SIM_DIR)/include
BIN       = conftest
OBJS      = entry.o \
            start.o \
            print.o \
            trap.o \
            sys.o \
            backtrace.o \
            symtable.o \
            tests/

EXTRA_CLEAN = .conftest* .symbols* symbols.S

#
# When linking a final binary, build in a symbol table.
#
USE_SYMTABLE = 1
