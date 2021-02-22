#
# Build a bootrom binary for the default r5 core.
#

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
