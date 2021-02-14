#
# Build a bootrom binary for the default r5 core.
#

# Turn off suffix rules. They are deprecated.
.SUFFIXES:

include $(R5SIM_DIR)/build/riscv-xc.mk

INCLUDES  = -I$(R5SIM_DIR)/include
LDFLAGS   += -T conftest.lds

BIN       = conftest

__OBJS    = entry.o \
            start.o \
            print.o \
            trap.o \
            sys.o \
	    backtrace.o \
            symtable.o \
            test_env.o \
	    test_system.o \
	    test_ldst.o \
	    test_muldiv.o \
	    test_op.o \
	    test_traps.o \
	    test_supervisor_traps.o

DEPFILES  = $(__OBJS:%.o=$(DEPDIR)/%.d)
DEPFILES  += $(DEPDIR)/symbols.d $(DEPDIR)/.symbols.d

OBJS      = conftest.o
# OBJS = $(__OBJS)

EXTRA_CLEAN = .conftest* .symbols* symbols.[So] $(__OBJS)

# Stage 1 link.
.conftest1.o: $(__OBJS)
	@echo "  [LD]\t\t$@"
	@$(LD) $(LDFLAGS) -r -o $@ $(__OBJS)

# Stage 1 symbol data - generate a C source file  with a list of all
# the symbols in the raw linked object.
.symbols.S: .conftest1.o
	@echo "  [GEN]\t\t$@"
	@$(NM) -n .conftest1.o | grep '\( [tT] \)' | grep -v ' \.' | \
		python3 syms.py > .symbols.S

# We need .symbols.o and conftest1.o to build our phase 2 linkage.
# .symbols.c is built as a prereq to .symbols.o.
.conftest2.o: .symbols.o
	@echo "  [LD]\t\t$@"
	@$(LD) $(LDFLAGS) -o $@ .conftest1.o .symbols.o

# We can now rebuild the symbol table, this time with the updated link
# from phase 2.
symbols.S: .conftest2.o
	@echo "  [GEN]\t\t$@"
	@$(NM) -n .conftest2.o | grep '\( [tT] \)' | grep -v ' \.' | \
		python3 syms.py > symbols.S

# Link 3:
conftest.o: .conftest2.o symbols.o
	@echo "  [LD]\t\t$@"
	@$(LD) $(LDFLAGS) -o $@ .conftest2.o symbols.o

include $(R5SIM_DIR)/build/Makefile.rules
