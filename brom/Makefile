#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:


include $(R5SIM_DIR)/build/riscv-xc.mk
LDFLAGS   += -T brom.lds

INCLUDES  = -I$(R5SIM_DIR)/include

OBJS      = brom.o start.o

BIN       = brom

include $(R5SIM_DIR)/build/Makefile.rules
