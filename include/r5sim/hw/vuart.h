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
 * VUART shared defines for the HW and for the "driver".
 */

#ifndef __R5SIM_HW_VUART_H__
#define __R5SIM_HW_VUART_H__

/*
 * Trivial UART mock: loads to offset VUART_READ will read a character
 * and stores to VUART_WRITE will write a character to the "UART".
 */

#define VUART_READ	0x0
#define VUART_WRITE	0x4

#endif
