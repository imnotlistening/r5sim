/*
 * Very simple BROM C code. This cannot write to the brom itself, so all
 * non-stack data is read only.
 *
 * We'll get a stack somewhere in memory. brom.S handles this.
 */

#include <r5sim/hw/vuart.h>
#include <r5sim/hw/vdisk.h>

typedef unsigned int uint32_t;

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
#define readl(addr)		*((uint32_t *)(addr))
#define writel(addr, val)	*((uint32_t *)(addr)) = (uint32_t)(val)

/*
 * For now just sit in an infinite loop.
 */
#define assert(cond)							\
	do {								\
		if (!cond) {						\
			while (1);					\
		}							\
	} while (0)

__attribute__((unused)) static char
brom_getc(void)
{
	return (char) readl(VUART_BASE + VUART_READ);
}

static void
brom_putc(char c)
{
	writel(VUART_BASE + VUART_WRITE, c);
}

static void
brom_puts(char *str)
{
	int i = 0;

	while (str[i] != 0)
		brom_putc(str[i++]);

	brom_putc('\r');
}

/*
 * Load a page from the vdisk and place it at dest.
 */
static void
load_disk_page(uint32_t dest, uint32_t offset)
{
	/*
	 * Configure op.
	 */
	writel(VDISK_BASE + VDISK_DRAM_ADDR,  (uint32_t)dest);
	writel(VDISK_BASE + VDISK_PAGE_START, offset);
	writel(VDISK_BASE + VDISK_PAGES,      1);

	/*
	 * Execute.
	 */
	writel(VDISK_BASE + VDISK_OP, VDISK_OP_COPY_TO_DRAM);
}

/*
 * Load a bootloader in the first 4KB of the vdisk.
 */
static void
load_bootloader(void)
{
	uint32_t vdisk_pgsz = readl(VDISK_BASE + VDISK_PAGE_SIZE);
	uint32_t dram_base = 0x40000000;
	uint32_t offset, page;
	blcall start;

	brom_puts("Loading bootloader to DRAM start @ 0x40000000\n");

	offset = 0;
	page = 0;

	while (offset < 4096) {
		load_disk_page(dram_base + offset, page);
		offset += vdisk_pgsz;
		page += 1;
	}

	start = (blcall) dram_base;
	start();
}

void
start(void)
{
	int vdisk_present;

	brom_getc();
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
