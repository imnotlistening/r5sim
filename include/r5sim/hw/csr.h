/*
 * CSR defines for r5sim and other entities.
 */

#ifndef __R5SIM_HW_CSR_H__
#define __R5SIM_HW_CSR_H__

/*
 * Basic CSRs.
 */
#define CSR_CYCLE		0xC00
#define CSR_TIME		0xC01
#define CSR_INSTRET		0xC02
#define CSR_CYCLEH		0xC80
#define CSR_TIMEH		0xC81
#define CSR_INSTRETH		0xC82

/*
 * Machine mode CSRs
 */
#define CSR_MSTATUS		0x300
#define CSR_MSTATUS_MPP		12:11
#define CSR_MSTATUS_MPIE	7:7
#define CSR_MSTATUS_MIE		3:3

#define CSR_MISA		0x301
#define CSR_MISA_MXL            31:30
#define CSR_MISA_EXTENSION      25:0

#define CSR_MEDELEG		0x302
#define CSR_MIDELEG		0x303

#define CSR_MIE			0x304
#define CSR_MIE_MSIE		3:3
#define CSR_MIE_MTIE		7:7

#define CSR_MTVEC		0x305
#define CSR_MTVEC_BASE		31:2
#define CSR_MTVEC_MODE		1:0
#define CSR_MTVEC_MODE_DIRECT	0
#define CSR_MTVEC_MODE_VECTORED	1

#define CSR_MSCRATCH		0x340
#define CSR_MEPC		0x341
#define CSR_MCAUSE		0x342
#define CSR_MCAUSE_INTERRUPT	31:31
#define CSR_MCAUSE_CODE		30:0
#define CSR_MCAUSE_CODE_USI	0
#define CSR_MCAUSE_CODE_SSI	1
#define CSR_MCAUSE_CODE_MSI	3

#define CSR_MCAUSE_CODE_UTI	4
#define CSR_MCAUSE_CODE_STI	5
#define CSR_MCAUSE_CODE_MTI	7

#define CSR_MCAUSE_CODE_UEI	8
#define CSR_MCAUSE_CODE_SEI	9
#define CSR_MCAUSE_CODE_MEI	11


#define CSR_MTVAL		0x343

#define CSR_MIP			0x344
#define CSR_MIP_MSIP		3:3

#define CSR_MVENDORID		0xF11
#define CSR_MARCHID		0xF21
#define CSR_MIMPID		0xF13
#define CSR_MHARTID		0xF14

#endif
