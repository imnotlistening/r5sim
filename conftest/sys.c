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
 * Print basic system info.
 */

#include <r5sim/hw/vsys.h>

#include <ct/csr.h>
#include <ct/types.h>
#include <ct/conftest.h>

/*
 * Base addresses for the default machine.
 */
#define VSYS_BASE	0x4000100

static const char *mxlen_to_str(u32 mxlen)
{
	static const char *mxlen_str[] = {
		"??",
		"32",
		"64",
		"128",
	};

	return mxlen_str[mxlen & 0x3];
}

static char extension_to_char(u32 ext)
{
	return 'a' + ext;
}

static u32 sys_readl(u32 offs)
{
	return readl(VSYS_BASE + offs);
}

static void print_core(void)
{
	u32 misa;
	u32 mxlen, extensions;
	u32 i;
	char e_buf[2] = { 0 };

	read_csr(CSR_MISA, misa);
	mxlen      = get_field(misa, CSR_MISA_MXL);
	extensions = get_field(misa, CSR_MISA_EXTENSION);

	/*
	 * Base core details.
	 */
	printf("  r5core: rv%s", mxlen_to_str(mxlen));

	/*
	 * I or E? Ahh, need to implement %c in printf()...
	 */
	e_buf[0] = extensions & (1 << 8) ? 'i' : 'e';
	printf("%s", e_buf);

	/*
	 * And now anything else, alphabetically.
	 */
	for (i = 0; i < 25; i++) {
		/* We've already handled I and E */
		if (i == 8)
			continue;

		if (extensions & (1 << i)) {
			e_buf[0] = extension_to_char(i);
			printf("%s", e_buf);
		}
	}

	printf("\n");
}

void ct_sys_info(void)
{
	u32 dram_start, dram_size;
	u32 io_start, io_size;

	printf("System info:\n");

	print_core();

	dram_start = sys_readl(VSYS_DRAM_START);
	dram_size  = sys_readl(VSYS_DRAM_START);
	io_start   = sys_readl(VSYS_IO_START);
	io_size    = sys_readl(VSYS_IO_SIZE);

	printf("  IO   base @ 0x%08x\n", io_start);
	printf("  IO   size @ 0x%08x\n", io_size);
	printf("  DRAM base @ 0x%08x\n", dram_start);
	printf("  DRAM size @ 0x%08x\n", dram_size);
}
