/*
 * Super simple BROM link script. We will place everything at address 0x10000
 * (e.g 64KB).
 */

SECTIONS
{
    /* Boot vector is 0x10000. */
    . = 0x10000;
    .text.boot : { *(.text.boot) }

    /* Now dump everything else we want. We have up to 4KB. */
    . = 0x10020;
    .text : { *(.text) }
    .rodata : { *(.rodata) *(.data) }
}
