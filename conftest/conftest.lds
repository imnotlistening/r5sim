/*
 * Super simple linker script to organize the conftest.
 */

SECTIONS
{
    /* DRAM is at 0x20000000. */
    . = 0x20000000;
    __start = . ;
    ENTRY(__start)
    .text.boot : { *(.text.boot) }

    /*
     * Now dump everything else we want. We have up to 16KB if we want to stay
     * within the page loaded by the BROM.
     */
    . = 0x20000020;
    .text : { *(.text) }
    .bss : { *(.bss) }
    .rodata : { *(.rodata) *(.data) }


    /*
     * Output the symbol table. Will be generated in phase two of the linking.
     * This needs to go at the end so that the original linking still is correct.
     *
     * .symtable.info is a small struct describing the latter .symtable. The
     * purpose of this struct is the avoid the need to build a relocatable output
     * during phase 1; if we do that the symbol addresses are all offset by a
     * little bit in the final output.
     *
     * Instead, this lets us resolve the symtable info struct during phase 1
     * linking. It's address won't change during phase 2, but the contents will
     * since there will now be an actual symbol table.
     */
    .symtable.header ALIGN(0x40) : {
        __symtable_header = . ;
        *(.rodata.symtable.header)
        __symtable_header_end = . ;
    }

    .symtable ALIGN(0x40) : {
        __symtable = . ;
        *(.rodata.symtable)
    }
}
