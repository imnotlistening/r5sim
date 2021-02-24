#
# Build a bootrom binary for the default r5 core.
#

include $(R5SIM_DIR)/build/riscv-xc.mk

BIN       = conftest
OBJS      = entry.o \
            start.o \
            print.o \
            trap.o \
            sys.o \
            backtrace.o \
            symtable.o \
            testexec.o \
            time.o \
            tests/

INCLUDES    += -I$(R5SIM_DIR)/include \
               -I$(R5SIM_DIR)/conftest/include
EXTRA_CLEAN += .conftest* .symbols* symbols.S

#
# When linking a final binary, build in a symbol table.
#
USE_SYMTABLE = 1
