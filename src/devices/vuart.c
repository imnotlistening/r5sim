/*
 * Define a virtual UART device that can be used for reading/writing
 * data into the R5.
 */

#include <pty.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include <r5sim/log.h>
#include <r5sim/env.h>
#include <r5sim/iodev.h>

#include <r5sim/hw/vuart.h>

struct virt_uart_priv {
	char fd_path[32];
	int  master_fd;
	int  slave_fd;
};

static u32 virt_uart_readl(struct r5sim_iodev *iodev, u32 offs)
{
	struct virt_uart_priv *priv = iodev->priv;
	char c = 0;

	r5sim_dbg_vv("vuart: R=%c (%u)\n", c, offs);

	if (offs != VUART_READ)
		return 0x0;

	/*
	 * Not a lot we can do if it fails.
	 */
	r5sim_assert(read(priv->master_fd, &c, 1));

	r5sim_dbg_vv("vuart: R=%c\n", c);

	return (u32)c;
}

static void virt_uart_writel(struct r5sim_iodev *iodev,
		 u32 offs, u32 val)
{
	struct virt_uart_priv *priv = iodev->priv;
	char c = val & 0xff;

	r5sim_dbg_vv("vuart: W=%c (%u)\n", c, offs);

	if (offs != VUART_WRITE)
		return;

	/*
	 * Not much we can do if it fails.
	 */
	r5sim_assert(write(priv->master_fd, &c, 1) == 1);
}

/*
 * Define the outlines for a virtual UART IODEV. Other fields will be
 * filled in on instantiation.
 */
static struct r5sim_iodev virtual_uart = {
	.name      = "vuart",

	.io_size   = 0x8,

	.readl     = virt_uart_readl,
	.writel    = virt_uart_writel,
};

struct r5sim_iodev *r5sim_vuart_load_new(struct r5sim_machine *mach,
					 u32 io_offs)
{
	struct r5sim_iodev *dev;
	struct virt_uart_priv *priv;

	dev = malloc(sizeof(*dev));
	r5sim_assert(dev != NULL);

	*dev = virtual_uart;

	priv = malloc(sizeof(*priv));
	r5sim_assert(priv != NULL);

	dev->mach = mach;
	dev->io_offset = io_offs;
	dev->priv = priv;

	if (openpty(&priv->master_fd,
		    &priv->slave_fd,
		    priv->fd_path,
		    NULL, NULL)) {
		perror("openpty");
		r5sim_assert(false);
        }

	r5sim_info("VUART @ 0x%x: pty=%s\n", io_offs, priv->fd_path);

	return dev;
}
