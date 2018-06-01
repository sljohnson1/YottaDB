/****************************************************************
 *								*
 * Copyright 2001, 2002 Sanchez Computer Associates, Inc.	*
 *								*
 * Copyright (c) 2017 YottaDB LLC. and/or its subsidiaries.	*
 * All rights reserved.						*
 *								*
 * Copyright (c) 2017 Stephen L Johnson. All rights reserved.	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

/*	aarch64.h - AArch64 machine instruction information.
 *
 *	Requires "aarch64_registers.h" and "aarch64_gtm_registers.h".
 *
 */


/*	Machine instruction templates.  */

#define	AARCH64_INS_ADD_IMM	((unsigned)0x91 << AARCH64_SHIFT_OP_24)
#define	AARCH64_INS_ADD_REG	((unsigned)0x8b << AARCH64_SHIFT_OP_24)
#define	AARCH64_INS_ADD_REG_EXT	((unsigned)0x8b2 << AARCH64_SHIFT_OP_20) | (AARCH64_OPTION_SXTW << AARCH64_SHIFT_OPTION)
#define AARCH64_INS_ADR		((unsigned)0x10 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_B		((unsigned)0x14 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_BEQ		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_EQ << AARCH64_SHIFT_COND))
#define AARCH64_INS_BGE		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_GE << AARCH64_SHIFT_COND))
#define AARCH64_INS_BGT		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_GT << AARCH64_SHIFT_COND))
#define AARCH64_INS_BL		((unsigned)0x94 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_BLE		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_LE << AARCH64_SHIFT_COND))
#define AARCH64_INS_BLR		((unsigned)0xd63f << AARCH64_SHIFT_OP_16)
#define AARCH64_INS_BLT		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_LT << AARCH64_SHIFT_COND))
#define AARCH64_INS_BNE		((unsigned)0x54 << AARCH64_SHIFT_OP_24 | (AARCH64_COND_NE << AARCH64_SHIFT_COND))
#define AARCH64_INS_BR		((unsigned)0xd61f << AARCH64_SHIFT_OP_16)
#define AARCH64_INS_CMN_IMM	((unsigned)0x31 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_CMN_REG	((unsigned)0xab << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_CMP_IMM	((unsigned)0x71 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_CMP_REG	((unsigned)0xeb << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_LDP		((unsigned)0xa8c << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_LDR_W	((unsigned)0xb94 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_LDR_X	((unsigned)0xf94 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_LDRSW	((unsigned)0xb98 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_LSR		((unsigned)0xd34 << AARCH64_SHIFT_OP_20) | (0xfc00)
#define AARCH64_INS_MOV_IMM	((unsigned)0x528 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_MOV_INV	((unsigned)0x128 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_MOV_REG	((unsigned)0xaa0 << AARCH64_SHIFT_OP_20) | (0x3e0)
#define	AARCH64_INS_MOVK	((unsigned)0x728 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_NOP		((unsigned)0xd503201f)
#define AARCH64_INS_STP		((unsigned)0xa88 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_STR_W	((unsigned)0xb90 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_STR_X	((unsigned)0xf90 << AARCH64_SHIFT_OP_20)
#define AARCH64_INS_SUB_IMM	((unsigned)0xd1 << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_SUB_REG	((unsigned)0xcb << AARCH64_SHIFT_OP_24)
#define AARCH64_INS_SUB		((unsigned)0x00 << AARCH64_SHIFT_OP_24)			/* xxxxxxx */


/* xxxxxxx
#define AARCH64_INS_TEQ		((unsigned)0xe13 << AARCH64_SHIFT_OP)
#define AARCH64_INS_TST		((unsigned)0xe31 << AARCH64_SHIFT_OP)
#define AARCH64_INS_ORRS		((unsigned)0xe19 << AARCH64_SHIFT_OP)
#define AARCH64_INS_ANDS		((unsigned)0xe21 << AARCH64_SHIFT_OP)
  xxxxxxx */

/* xxxxxxx #define AARCH64_REG2IMM_BIT		((unsigned)0x02 << AARCH64_SHIFT_OP_24) */	/* Converts ADD/SUB register to ADD/SUB immediate */

/* xxxxxxx
#define AARCH64_INS_BIS		((unsigned)0xc << AARCH64_SHIFT_DP | AARCH64_COND_ALWAYS << AARCH64_SHIFT_COND)
#define AARCH64_INS_BLBS		((unsigned)0x3c << AARCH64_SHIFT_OP)
   xxxxxxx */

#if 0		/* xxxxxxx */
/* Branch to Subroutine */
#define AARCH64_INS_BSR		((unsigned)0x34 << AARCH64_SHIFT_OP)
/* Jump */
#define	AARCH64_INS_JMP		((unsigned)0x1a << AARCH64_SHIFT_OP)

#define	AARCH64_INS_JSR		((unsigned)0x1a << AARCH64_SHIFT_OP | 1 << AARCH64_SHIFT_BRANCH_FUNC)
/* Load Sign-Extended Longword from Memory to Register */
#define	AARCH64_INS_LDL		((unsigned)0x01 << AARCH64_SHIFT_OP)
/* Load Quadword from Memory to Register */
#define	AARCH64_INS_LDQ		((unsigned)0x29 << AARCH64_SHIFT_OP)
#define	AARCH64_INS_RET		((unsigned)0x1a << AARCH64_SHIFT_OP | 2 << AARCH64_SHIFT_BRANCH_FUNC)
/* Store Longword from Register to Memory */
#define	AARCH64_INS_STL		((unsigned)0x2c << AARCH64_SHIFT_OP)
/* Store Quadword from Register to Memory */
#define	AARCH64_INS_STQ		((unsigned)0x2d << AARCH64_SHIFT_OP)
/* Subtract Longword */
#define	AARCH64_INS_SUBL		((unsigned)0x2 << AARCH64_SHIFT_DP)
/* Subtract Quadword */
#define	AARCH64_INS_SUBQ		((unsigned)0x10 << AARCH64_SHIFT_OP | 0x29 << AARCH64_SHIFT_FUNC)

#define AARCH64_INS_DP_IMMED	((unsigned)0x1 << AARCH64_SHIFT_I_BIT)
#endif			/* xxxxxxx */

#define AARCH64_SHIFT_TYPE_SHIFT	22
#define AARCH64_SHIFT_TYPE_LSL		0

#define AARCH64_OPTION_SXTW		0x6

/*	Bit offsets to instruction fields.  */

#define AARCH64_SHIFT_OP		24

#define AARCH64_SHIFT_OP_24		24
#define AARCH64_SHIFT_OP_20		20
#define AARCH64_SHIFT_OP_16		16

#define AARCH64_SHIFT_COND		0

#define AARCH64_SHIFT_RN		5
#define	AARCH64_SHIFT_RD		0
#define	AARCH64_SHIFT_RM		16
#define	AARCH64_SHIFT_RT		0
#define	AARCH64_SHIFT_RT2		10

#define AARCH64_SHIFT_IMM3		3
#define AARCH64_SHIFT_IMM6		10
#define AARCH64_SHIFT_IMMR		16
#define AARCH64_SHIFT_IMM7		15
#define AARCH64_SHIFT_IMM12		10
#define AARCH64_SHIFT_IMM16		5
#define AARCH64_SHIFT_BRANCH_DISP_COND	5
#define AARCH64_SHIFT_BRANCH_DISP	0
#define AARCH64_SHIFT_OPTION		13

/*	Bit masks for instruction fields. xxxxxxx  */

#define AARCH64_MASK_BRANCH_DISP	0x3ffffff
#define AARCH64_MASK_BRANCH_DISP_COND	0x7ffff

#define	AARCH64_MASK_OP			0xff		/* 8 bit opcode */
#define	AARCH64_MASK_OP_8		0xff		/* 8 bit opcode */
#define	AARCH64_MASK_OP_12		0xfff		/* 12 bit opcode */
#define	AARCH64_MASK_REG		0x1f		/* register */
#define AARCH64_MASK_COND		0xf		/* conditional */

#define AARCH64_MASK_SHIFT_TYPE		0x3

#define AARCH64_MASK_IMM3		0x3
#define AARCH64_MASK_IMM6		0x3f
#define AARCH64_MASK_IMMR		0x3f
#define AARCH64_MASK_IMM7		0x7f
#define AARCH64_MASK_IMM12		0xfff
#define AARCH64_MASK_IMM16		0xffff

#define AARCH64_COND_EQ			0x0
#define AARCH64_COND_NE			0x1
#define AARCH64_COND_GE			0xa
#define AARCH64_COND_LT			0xb
#define AARCH64_COND_GT			0xc
#define AARCH64_COND_LE			0xd
#define AARCH64_COND_ALWAYS		0xe

#define AARCH64_SF_BIT_SHIFT		31
#define AARCH64_32_BIT_OP		0x0 << AARCH64_SF_BIT_SHIFT
#define AARCH64_64_BIT_OP		0x1 << AARCH64_SF_BIT_SHIFT

#define AARCH64_HW_SHIFT		21
#define AARCH64_HW_SHIFT_16		0x1 << AARCH64_HW_SHIFT
#define AARCH64_HW_SHIFT_32		0x2 << AARCH64_HW_SHIFT
#define AARCH64_HW_SHIFT_48		0x3 << AARCH64_HW_SHIFT

#ifdef DEBUG
#define GET_OPCODE(ains)		((ains >> AARCH64_SHIFT_OP) & AARCH64_MASK_OP)
#define GET_RT(ains)			((ains >> AARCH64_SHIFT_RT) & AARCH64_MASK_REG)
#define GET_RT2(ains)			((ains >> AARCH64_SHIFT_RT2) & AARCH64_MASK_REG)
#define GET_RD(ains)			((ains >> AARCH64_SHIFT_RD) & AARCH64_MASK_REG)
#define GET_RN(ains)			((ains >> AARCH64_SHIFT_RN) & AARCH64_MASK_REG)
#define GET_RM(ains)			((ains >> AARCH64_SHIFT_RM) & AARCH64_MASK_REG)
#define GET_MEMDISP(ains)		((ains >> AARCH64_SHIFT_DISP) & AARCH64_MASK_DISP)
#define GET_REGISTERS(ains)		(ains & AARCH64_MASK_REGISTERS)
#define GET_IMM3(ains)			((ains >> AARCH64_SHIFT_IMM3) & AARCH64_MASK_IMM3)
#define GET_IMM6(ains)			((ains >> AARCH64_SHIFT_IMM6) & AARCH64_MASK_IMM6)
#define GET_IMMR(ains)			((ains >> AARCH64_SHIFT_IMMR) & AARCH64_MASK_IMMR)
#define GET_IMM7(ains)			((ains >> AARCH64_SHIFT_IMM7) & AARCH64_MASK_IMM7)
#define GET_IMM12(ains)			((ains >> AARCH64_SHIFT_IMM12) & AARCH64_MASK_IMM12)
#define GET_IMM16(ains)			((ains >> AARCH64_SHIFT_IMM16) & AARCH64_MASK_IMM16)
#define GET_COND(ains)			((ains >> AARCH64_SHIFT_COND) & AARCH64_MASK_COND)
#define GET_BRDISPCOND(ains)		((ains >> AARCH64_SHIFT_BRANCH_DISP_COND) & AARCH64_MASK_BRANCH_DISP_COND)
#define GET_BRDISP(ains)		((ains >> AARCH64_SHIFT_BRANCH_DISP) & AARCH64_MASK_BRANCH_DISP)

#define ADD_INST	"add"
#define ADR_INST	"adr"
#define B_INST		"b"
#define BEQ_COND	"b.eq"
#define BGE_COND	"b.ge"
#define BGT_COND	"b.gt"
#define BL_INST		"bl"
#define BLE_COND	"b.le"
#define BLT_COND	"b.lt"
#define BLR_INST	"blr"
#define BNE_COND	"b.ne"
#define BR_INST		"br"
#define CMN_INST	"cmn"
#define CMP_INST	"cmp"
#define LDP_INST	"ldp"
#define LDR_INST	"ldr"
#define LDRSW_INST	"ldrsw"
#define LSR_INST	"lsr"
#define MOV_INST	"mov"
#define MOVK_INST	"movk"
#define NOP_INST	"nop"
#define STP_INST	"stp"
#define STR_INST	"str"
#define SUB_INST	"sub"
#define INV_INST	"<invalid instruction>"

#define LSL_SHIFT_TYPE	"lsl"

/* Space for op_code to be in */
#define OPSPC		7
#endif
