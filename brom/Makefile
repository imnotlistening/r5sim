#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc
LD        = riscv64-linux-gnu-ld

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32i -mabi=ilp32
LDFLAGS   = -melf32lriscv -nostartfiles -nostdlib -nodefaultlibs -T brom.lds

INCLUDES  = -I$(R5SIM_DIR)/include

OBJS      = brom.o start.o

.PHONY: all clean install

all: brom.bin
	@echo > /dev/null

install:
	@riscv64-linux-gnu-objcopy -O binary brom.bin $(INSTALL)/brom

clean:
	@rm -rf $(OBJS) $(BINS) .deps

.PRECIOUS: $(OBJS)

include $(R5SIM_DIR)/build/Makefile.rules
