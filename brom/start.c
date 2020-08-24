/*
 * Very simple BROM C code. This cannot write to the brom itself, so all
 * non-stack data is read only.
 *
 * We'll get a stack somewhere in memory. brom.S handles this.
 */

typedef unsigned int uint32_t;

/*
 * VUART definitions. Very simple!
 */
#define VUART_BASE	0x4000000
#define VUART_READ	0x0
#define VUART_WRITE	0x4

/*
 * No need for barriers on the simple_core.
 */
#define readl(addr)		*((uint32_t *)(addr))
#define writel(addr, val)	*((uint32_t *)(addr)) = (uint32_t)(val)

static char brom_getc(void)
{
	return (char) readl(VUART_BASE + VUART_READ);
}

static void brom_putc(char c)
{
	writel(VUART_BASE + VUART_READ, c);
}

static void brom_puts(char *str)
{
	int i = 0;

	while (str[i] != 0)
		brom_putc(str[i++]);
}

void start(void)
{
	brom_getc();
	brom_puts("Hello! Welcome to the r5sim!\n\r");
	brom_puts("  This actually works!? Wow!\n\r");
}
