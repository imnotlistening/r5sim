#
# Build the simulator proper.
#

INCLUDES  += -I$(R5SIM_DIR)/include
LIBS      += -lutil -lrt -lpthread -lreadline
CFLAGS    += -ggdb -O2 -Wall -rdynamic

OBJS      = r5sim.o \
	    iodev.o \
            machine.o \
            log.o \
            core.o \
            core_intr.o \
            csr.o \
            simple_core.o \
            vuart.o \
            vdisk.o \
            vsys.o

# Subdirectories.
OBJS      += debugger/

APP       = r5sim
