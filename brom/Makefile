#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32i -mabi=ilp32    \
             -Wl,--build-id=none -nostartfiles -nodefaultlibs -T brom.lds

INCLUDES  =
LIBS      =

BINS      = brom
OBJS      = start.o

.PHONY: all clean install

all: $(BINS)
	@echo > /dev/null

install:
	@for b in $(BINS); do \
		riscv64-linux-gnu-objcopy -O binary $$b $(INSTALL)/$$b ; \
	done

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
