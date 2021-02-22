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
            test_env.o \
            test_system.o \
            test_ldst.o \
            test_muldiv.o \
            test_op.o \
            test_traps.o \
            test_supervisor_traps.o

EXTRA_CLEAN = .conftest* .symbols* symbols.S
