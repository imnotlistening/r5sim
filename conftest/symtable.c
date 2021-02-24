/*
 * Some routines to search and describe the auto-generated symbol table.
 */

#include <ct/symbols.h>
#include <ct/conftest.h>

extern u32 __symtable;
extern u32 __symtable_header;

static void slowdown()
{
	volatile u32 x = 0;

	while (x++ < 1)
		;
}

void __read_symtable_info(struct symbol **symtable, u32 *length)
{
	struct symbol *syms = (struct symbol *)&__symtable;
	volatile u32 *header = &__symtable_header;

	*length   = *header;
	*symtable = syms;
}

void symtable_describe(void)
{
	u32 length, i;
	struct symbol *symtable;

	__read_symtable_info(&symtable, &length);

	printf("Symbol table info:\n");
	printf("  Start: 0x%08x\n", (u32)symtable);
	printf("  Nr:    %u\n",     length);

	for (i = 0; i < length; i++) {
		volatile struct symbol *sym = &symtable[i];

		printf("Sym %3u: 0x%08x %s\n", i, sym->addr, sym->sym);
	}
}

const char *addr2sym(u32 addr, u32 *offset)
{
	u32 length;
	u32 lo, hi, mid = 0;
	struct symbol *symtable;

	__read_symtable_info(&symtable, &length);

	lo = 0;
	hi = length;

	/*
	 * Binary search for the symbol that this address corresponds to.
	 */
	while ((hi - lo) != 1u) {
		struct symbol *mid_sym;

		mid = (lo + hi) >> 1;
		mid_sym = &symtable[mid];

		if (mid_sym->addr > addr)
			hi = mid;
		else
			lo = mid;

		slowdown();
	}

	*offset = addr - symtable[lo].addr;

	return symtable[lo].sym;
}
