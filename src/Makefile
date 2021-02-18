#
# Build the simulator proper.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

INCLUDES  = -I$(R5SIM_DIR)/include
LIBS      = -lutil -lrt -lpthread -lreadline
CFLAGS    += -ggdb -O2 -Wall

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
            vsys.o \
            hwdebug.o \
            session.o

APP       = r5sim

include $(R5SIM_DIR)/build/Makefile.rules
