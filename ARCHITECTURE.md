# R5SIM Architecture

There's really two things to think about with the simulator architecture: the
machine and the RISC-V core.

# r5sim Default Machine Architecture

Although it's possible to define your own machine architecture, the default
architecture provided by r5sim should be sufficient for some super simple
programs.

`r5sim` is a 32 bit simulator, so the maximum available address space is
4GB. It's currently, by default, partitioned like so (not to scale):

    +---------------+ 0x0
    |               |
    +---------------+ 0x10000    | BROM Base (size=0x4KB)
    |               |
    +---------------+ 0x11000    | End of BROM
    |               |
    +---------------+ 0x4000000  | IO Aperture Base (size=32MB)
    |               |
    |               |
    +---------------+ 0x6000000  | End of IO Aperture
    |               |
    +---------------+ 0x40000000 | DRAM Base (size=256MB)
    |               |
    |               |
    |               |
    +---------------+ 0x50000000 | End of DRAM
    |    ...        |

## Boot Flow

`r5sim` loads the bottom 4KB of whatever file is passed as the BROM into
the BROM memory region. Once this is complete, the r5sim machine begins
execution in the default r5sim core implementation at PC=0x10000.

At this point you have all registers set to 0x0, and all of memory cleared
to zero.

## Endianness

Conveniently x86_64 (the typical host arch) and RISC-V share the same little
endianness. As such there's really not much to worry about here - for now.

# Memory Model

The memory model provided by default in `r5sim` is essentially a strongly
ordered, no cache model. That is every load and store is completely coherent
with one another - including accesses to IO memory. Therefore the fence
instructions (i.e barriers) are noops in the simple core; This could be
modeled by the machine at some point, as an interesting testing mechanism.
Also, there is no SMP for now.

## BROM

Only loads are allowed in the BROM region. Any stores will cause the
simulator to assert.

## MMIO Devices

`r5sim` supports Memory Mapped IO (MMIO) devices. Any load or store that is
sent to the MMIO region is routed to the relevant IO device at that address.
If no IO address is present then stores are dropped and loads return 0x0.
