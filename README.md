# R5SIM - Overview

`r5sim` is a very simple RISC-V simulator. Currently it supports the rv32i -
minus the SYSTEM instructions - RISC-V standard. It has support for a
trivially simple virtual UART (vuart) and a 4KB bootrom image.

This is enough to run some simple code BROM esque code. See the brom directory!

# Dependencies and Requirements

To run this you'll need a GCC host compiler, a RISC-V cross compiler and
GNU make. At least on Ubuntu these are easy enough to get:

    $ sudo apt-get install gcc
    $ sudo apt-get install gcc-riscv64-linux-gnu
    $ sudo apt-get install make

To build just a simple `make` should be sufficient.

# Run It

To run the default build do the following:

    $ bin/r5sim -b bin/brom

This will execute the code found in the brom binary.

# Architecture Overview

See ARCHITECURE.md for a description of the default architecture.
