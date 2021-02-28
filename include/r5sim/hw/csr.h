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
#define CSR_MSTATUS_SPP		8:8
#define CSR_MSTATUS_MPIE	7:7
#define CSR_MSTATUS_SPIE	5:5
#define CSR_MSTATUS_MIE		3:3
#define CSR_MSTATUS_SIE		1:1

#define CSR_MISA		0x301
#define CSR_MISA_MXL            31:30
#define CSR_MISA_EXTENSION      25:0

#define CSR_MEDELEG		0x302
#define CSR_MIDELEG		0x303

#define CSR_MIE			0x304
#define CSR_MIE_MTIE		7:7
#define CSR_MIE_STIE		5:5
#define CSR_MIE_MSIE		3:3
#define CSR_MIE_SSIE		1:1

#define CSR_MIP			0x344
#define CSR_MIP_MTIP		7:7
#define CSR_MIP_STIP		5:5
#define CSR_MIP_MSIP		3:3
#define CSR_MIP_SSIP		1:1

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

#define CSR_MVENDORID		0xF11
#define CSR_MARCHID		0xF21
#define CSR_MIMPID		0xF13
#define CSR_MHARTID		0xF14

/*
 * PMP config registers.
 */
#define CSR_PMPCFG0		0x3A0
#define CSR_PMPCFG1		0x3A1
#define CSR_PMPCFG2		0x3A2
#define CSR_PMPCFG3		0x3A3

#define CSR_PMPCFG_L		7:7
#define CSR_PMPCFG_A		4:3
#define CSR_PMPCFG_X		2:2
#define CSR_PMPCFG_W		1:1
#define CSR_PMPCFG_R		0:0

#define CSR_PMPADDR0		0x3B0
#define CSR_PMPADDR1		0x3B1
#define CSR_PMPADDR2		0x3B2
#define CSR_PMPADDR3		0x3B3
#define CSR_PMPADDR4		0x3B4
#define CSR_PMPADDR5		0x3B5
#define CSR_PMPADDR6		0x3B6
#define CSR_PMPADDR7		0x3B7
#define CSR_PMPADDR8		0x3B8
#define CSR_PMPADDR9		0x3B9
#define CSR_PMPADDR10		0x3BA
#define CSR_PMPADDR11		0x3BB
#define CSR_PMPADDR12		0x3BC
#define CSR_PMPADDR13		0x3BD
#define CSR_PMPADDR14		0x3BE
#define CSR_PMPADDR15		0x3BF

/*
 * Supervisor CSRs.
 */
#define CSR_SSTATUS		0x100
#define CSR_SSTATUS_SPP		8:8
#define CSR_SSTATUS_SPIE	5:5
#define CSR_SSTATUS_SIE		1:1

#define CSR_SIE			0x104
#define CSR_SIE_STIE		5:5
#define CSR_SIE_SSIE		1:1

#define CSR_SIP			0x144
#define CSR_SIP_STIP		5:5
#define CSR_SIP_SSIP		1:1

#define CSR_STVEC		0x105
#define CSR_SSCRATCH		0x140
#define CSR_SEPC		0x141

#define CSR_SCAUSE		0x142
#define CSR_SCAUSE_INTERRUPT	31:31
#define CSR_SCAUSE_CODE		30:0

#define CSR_STVAL		0x143


#endif
