/*
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
