/* Copyright 2021, Alex Waterman <imnotlistening@gmail.com>
 *
 * This file is part of r5sim.
 *
 * r5sim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * r5sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with r5sim.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Very simple BROM C code. This cannot write to the brom itself, so all
 * non-stack data is read only.
 *
 * We'll get a stack somewhere in memory. brom.S handles this.
 */

#include <r5sim/hw/vuart.h>
#include <r5sim/hw/vdisk.h>

typedef unsigned int u32;

typedef void (*blcall)(void);

/* DRAM base. We'll load the bootloader there. */

/*
 * Base addresses for the default machine.
 */
#define VUART_BASE	0x4000000
#define VDISK_BASE	0x4001000

/*
 * No need for barriers on the simple_core.
 */
#define readl(addr)		*((volatile u32 *)(addr))
#define writel(addr, val)	*((volatile u32 *)(addr)) = (u32)(val)

/*
 * For now just sit in an infinite loop.
 */
#define assert(cond)							\
	do {								\
		if (!cond) {						\
			while (1);					\
		}							\
	} while (0)

__attribute__((unused))
static char brom_getc(void)
{
	return (char) readl(VUART_BASE + VUART_READ);
}

static void brom_putc(char c)
{
	writel(VUART_BASE + VUART_WRITE, c);
}

static void brom_puts(char *str)
{
	int i = 0;

	while (str[i] != 0)
		brom_putc(str[i++]);

	brom_putc('\r');
}

/*
 * Load a page from the vdisk and place it at dest.
 */
static void load_disk_page(u32 dest, u32 offset)
{
	/*
	 * Configure op.
	 */
	writel(VDISK_BASE + VDISK_DRAM_ADDR,  (u32)dest);
	writel(VDISK_BASE + VDISK_PAGE_START, offset);
	writel(VDISK_BASE + VDISK_PAGES,      1);
	writel(VDISK_BASE + VDISK_OP,         VDISK_OP_COPY_TO_DRAM);

	/*
	 * Execute.
	 */
	writel(VDISK_BASE + VDISK_EXEC, 1);

	/*
	 * Poll till done.
	 */
	while (readl(VDISK_BASE + VDISK_BUSY))
		;
}

/*
 * Load a bootloader in the first 16KB of the vdisk.
 */
static void load_bootloader(void)
{
	u32 vdisk_pgsz = readl(VDISK_BASE + VDISK_PAGE_SIZE);
	u32 dram_base = 0x20000000;
	u32 offset, page;
	blcall start;

	brom_puts("Loading bootloader to DRAM start @ 0x20000000\n");

	offset = 0;
	page = 0;

	while (offset < (64 << 10)) {
		load_disk_page(dram_base + offset, page);
		offset += vdisk_pgsz;
		page += 1;
	}

	start = (blcall) dram_base;
	start();
}

void start(void)
{
	int vdisk_present;

#ifdef CONFIG_PAUSE_B4_LOAD
	brom_getc();
#endif
	brom_puts("Hello! Welcome to the r5sim!\n");

	vdisk_present = readl(VDISK_BASE + VDISK_PRESENT);
	if (!vdisk_present) {
		brom_puts("No disk present! Cannot load bootloader.\n");
		assert(0);
	}

	/*
	 * Otherwise we have a disk - try and load a bootloader.
	 */
	load_bootloader();
}
