#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

include $(R5SIM_DIR)/build/riscv-xc.mk

LDFLAGS   += -T conftest.lds
INCLUDES  = -I$(R5SIM_DIR)/include

BIN       = conftest

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

include $(R5SIM_DIR)/build/Makefile.rules
