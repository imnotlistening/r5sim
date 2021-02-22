#
# Some config settings for building this project.
#

include $(R5SIM_DIR)/build/riscv-xc.mk

#
# When linking a final binary, build in a symbol table.
#
USE_SYMTABLE = 1
