# R5SIM - Overview

`r5sim` is a very simple RISC-V simulator. Currently it supports the rv32im
RISC-V standard. It has support for a trivially simple virtual UART (vuart)
and small bootrom image.

The BROM loads and executes a 16KB binary - a bootloader of sorts. Currently
the only code the BROM has to load is a WIP conformance test for the
simple-core implementation.

# Dependencies and Requirements

To run this you'll need a GCC host compiler, a RISC-V cross compiler and
GNU make. At least on Ubuntu these are easy enough to get:

    $ sudo apt-get install gcc gcc-riscv64-linux-gnu
    $ sudo apt-get install libreadline-dev
    $ sudo apt-get install make

To build just a simple `make` should be sufficient. This will produce a few
binaries under bin/.

# Run It

To run the default build and conformance test do the following:

    $ bin/r5sim -b bin/brom -d bin/conftest

This will execute the code found in the brom binary. The BROM binary just
loads the first 16KB from the VDISK into memory at the start of DRAM.
The BROM then jumps to the base of DRAM.

To see actual output you'll need a terminal program (GNU screen, minicom,
picocom, etc). The VUART creates a pseudo terminal which can be used to
get access to the r5sim machine's "UART". A typical run will look something
like this:

```
$ ./bin/r5sim -b bin/brom -d bin/conftest
[I]         vuart.c:103  | VUART @ 0x0: pty=/dev/pts/36
[I]         vdisk.c:235  | VDISK @ 0x1000: path=bin/conftest
[I]         vdisk.c:236  |   Disk size: 5020
[I]       machine.c:399  | Machine description: default-r5
[I]       machine.c:401  |   DRAM:        0x40000000 + 0x10000000)
[I]       machine.c:403  |   IO Aperture: 0x04000000 + 0x02000000)
...
```

The PTY path is printed by the VUART module. Connect to this path like so:

```
$ picocom /dev/pts/36
```

At this point the default BROM has not actually executed anything yet. The
BROM waits for a character to be received on the VUART before loading any
binary found on the VDISK. This allows you to actually see the VDISK program
execute.

# Architecture Overview

See ARCHITECURE.md for a description of the default architecture.
