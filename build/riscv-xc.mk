#
# RISC-V specifics for building risc-v targeted binaries.
#

# Overwrite the CC to a riscv X-compiler.
CC        = riscv64-linux-gnu-gcc
LD        = riscv64-linux-gnu-ld
OBJCOPY   = riscv64-linux-gnu-objcopy

# Add some special R5 options!
CFLAGS    += -ffreestanding -march=rv32im -mabi=ilp32 -O2
LDFLAGS   = -melf32lriscv -nostartfiles -nostdlib -nodefaultlibs