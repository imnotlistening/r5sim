#
# Build the simulator proper.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

INCLUDES  = -I$(R5SIM_DIR)/include
LIBS      = -lutil

BINS      = r5sim
OBJS      = iodev.o machine.o vuart.o vdisk.o log.o core.o csr.o simple_core.o

CFLAGS    += -ggdb -O2 -Wall

.PHONY: all clean install

all: $(BINS) $(TESTS)
	@echo > /dev/null

install:
	@cp $(BINS) $(INSTALL)

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
