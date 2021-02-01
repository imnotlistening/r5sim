#
# Build the simulator proper.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

INCLUDES  = -I$(R5SIM_DIR)/include
LIBS      = -lutil -lrt

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

CFLAGS    += -ggdb -O2 -Wall

.PHONY: all clean install

all: r5sim
	@echo > /dev/null

install:
	@cp r5sim $(INSTALL)

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
