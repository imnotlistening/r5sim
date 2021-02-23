#
# Invoke this makefile to do a recursive make invocation in the requested
# directory. The expected usage is:
#
#   $(MAKE) -C <subdir> -f $(R5SIM_DIR)/build/recurse.mk <target>
#
# This will automatically include the necessary makefiles to pull in rules
# and other env stuff.
#

include $(R5SIM_DIR)/build/Makefile.defaults

# Include the actual Makefile. This'll define OBJS, etc.
-include Makefile

# And now include the rules. We want these to come in _after_ the objs,
# etc, are defined sicne this makes defining the targets a lot easier.
include $(R5SIM_DIR)/build/Makefile.rules
