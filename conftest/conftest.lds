/*
 * Super simple linker script to organize the conftest.
 */

SECTIONS
{
    /* DRAM is at 0x20000000. */
    . = 0x20000000;
    .text.boot : { *(.text.boot) }

    /*
     * Now dump everything else we want. We have up to 4KB if we want to stay
     * within the page loaded by the BROM.
     */
    . = 0x20000020;
    .text : { *(.text) }
    .rodata : { *(.rodata) *(.data) }
}
