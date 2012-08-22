/*	$Id: macdefs.h,v 1.1 2010/01/15 15:19:51 cdidier Exp $	*/
/*
 * Copyright (c) 2009 Colin Didier <cdidier@cybione.org>
 * Copyright (c) 2009 Thomas Guillem <thomas.guillem@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Machine-dependent defines for both passes.
 */

/*
 * Convert (multi-)character constant to integer.
 */
#define makecc(val,i)	lastcon = (lastcon<<8)|((val<<8)>>8);

/* XXX */
#define ARGINIT		(4*8)	/* # bits above fp where arguments start */
#define AUTOINIT	0	/* # bits above fp where automatics start */

/*
 * Storage space requirements
 */
#define SZCHAR		8
#define SZBOOL		8
#define SZINT		16
#define SZFLOAT		32
#define SZDOUBLE	32
#define SZLDOUBLE	64
#define SZLONG		32
#define SZSHORT		16
#define SZLONGLONG	64
#define SZPOINT(t)	16

/*
 * Alignment constraints: no alignment
 */
#define ALCHAR		8
#define ALBOOL		8
#define ALINT		8
#define ALFLOAT		8
#define ALDOUBLE	8
#define ALLDOUBLE	8
#define ALLONG		8
#define ALLONGLONG	8
#define ALSHORT		8
#define ALPOINT		8
#define ALSTRUCT	8
#define ALSTACK		8 

/*
 * Min/max values
 */
#define	MIN_CHAR	-128
#define	MAX_CHAR	127
#define	MAX_UCHAR	255
#define	MIN_SHORT	-32768
#define	MAX_SHORT	32767
#define	MAX_USHORT	255
#define	MIN_INT		-32768
#define	MAX_INT		32767
#define	MAX_UNSIGNED	65535
#define	MIN_LONG	-1
#define	MAX_LONG	0x7fffffff
#define	MAX_ULONG	0xffffffff
#define	MIN_LONGLONG	0x8000000000000000LL
#define	MAX_LONGLONG	0x7fffffffffffffffLL
#define	MAX_ULONGLONG	0xffffffffffffffffULL

#define	BOOL_TYPE	CHAR	/* what used to store _Bool */

/*
 * Use large-enough types.
 */
typedef	long long CONSZ;
typedef	unsigned long long U_CONSZ;
typedef long long OFFSZ;

#define CONFMT	"%lld"		/* format for printing constants */
#define LABFMT	".L%d"		/* format for printing labels */
#define	STABLBL	"LL%d"		/* format for stab (debugging) labels */

#define STAB_LINE_ABSOLUTE	/* S_LINE fields use absolute addresses */

/* Definitions mostly used in pass2 */

#define BYTEOFF(x)	1	/* XXX Not sure, to ask ! */
#define BITOOR(x)	(x)	/* bit offset to oreg offset XXX die! */

#define STOARG(p)
#define STOFARG(p)
#define STOSTARG(p)

#define szty(t) ((t) == LONGLONG || (t) == ULONGLONG			\
		|| (t) == LDOUBLE ? 8 :					\
		(t) == LONG || (t) == ULONG				\
		|| (t) == FLOAT || (t) == DOUBLE ? 4 :			\
		((t) <= UNSIGNED && (t) >= SHORT) || ISPTR(t) ? 2 : 1)

/* 8 bits registers (not used) */
#define R0	0
#define R1	1

/* 8 bits PERMREG */
#define R2	2
#define R3	3
#define R4	4
#define R5	5
#define R6	6
#define R7	7
#define R8	8
#define R9	9
#define R10	10
#define R11	11
#define R12	12
#define R13	13
#define R14	14
#define R15	15
#define R16	16
#define R17	17

/* 8 bits TEMPREG */
#define R18	18
#define R19	19
#define R20	20
#define R21	21
#define R22	22
#define R23	23
#define R24	24
#define R25	25

/* Overlaped by RX, RY, RZ (not used) */
#define R26	26
#define R27	27
#define R28	28
#define R29	29
#define R30	30
#define R31	31

/* Address registers */
#define RX	32	/* RX = R27:R26 */
#define RY	33	/* RY = R29:R28 */
#define RZ	34	/* RZ = R31:R30 */

/* 16 bits registers (not used) */
#define R0R1	35
#define R1R2	36

/* 16 bits PERMREG */
#define R2R3	37
#define R3R4	38
#define R4R5	39
#define R5R6	40
#define R6R7	41
#define R7R8	42
#define R8R9	43
#define R9R10	44
#define R10R11	45
#define R11R12	46
#define R12R13	47
#define R13R14	48
#define R14R15	49
#define R15R16	50
#define R16R17	51
#define R17R18	52

/* 16 bits TEMPREG */
#define R18R19	53
#define R19R20	54
#define R20R21	55
#define R21R22	56
#define R22R23	57
#define R23R24	58
#define R24R25	59

#define NUMCLASS	2
#define MAXREGSA	24
#define MAXREGSB	23
#define	MAXREGS		60

/* The classes used on avr are:
 *	A - 8-bit registers
 *	B - 16-bit registers
 */

#define RSTATUS \
/*0*/	0, 0, SAREG|PERMREG, SAREG|PERMREG,				\
/*4*/	SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG,	\
/*8*/	SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG,	\
/*12*/	SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG, SAREG|PERMREG,	\
/*16*/	SAREG|PERMREG, SAREG|PERMREG, SAREG|TEMPREG, SAREG|TEMPREG,	\
/*20*/	SAREG|TEMPREG, SAREG|TEMPREG, SAREG|TEMPREG, SAREG|TEMPREG,	\
/*24*/	SAREG|TEMPREG, SAREG|TEMPREG, 0, 0,				\
/*28*/	0, 0, 0, 0,							\
/*32*/	0, 0, 0, 0,							\
/*36*/	0, SBREG, SBREG, SBREG,		\
/*40*/	SBREG, SBREG, SBREG, SBREG,	\
/*44*/	SBREG, SBREG, SBREG, SBREG,	\
/*48*/	SBREG, SBREG, SBREG, SBREG,	\
/*52*/	SBREG, SBREG, SBREG, SBREG,	\
/*56*/	SBREG, SBREG, SBREG, SBREG

#define ROVERLAP \
/*0*/	{ R0R1, -1 },							\
/*1*/	{ R0R1, R1R2, -1 },						\
/*2*/	{ R1R2, R2R3, -1 },						\
/*3*/	{ R2R3, R3R4, -1 },						\
/*4*/	{ R3R4, R4R5, -1 },						\
/*5*/	{ R4R5, R5R6, -1 },						\
/*6*/	{ R5R6, R6R7, -1 },						\
/*7*/	{ R6R7, R7R8, -1 },						\
/*8*/	{ R7R8, R8R9, -1 },						\
/*9*/	{ R8R9, R9R10, -1 },						\
/*10*/	{ R9R10, R10R11, -1 },						\
/*11*/	{ R10R11, R11R12, -1 },						\
/*12*/	{ R11R12, R12R13, -1 },						\
/*13*/	{ R12R13, R13R14, -1 },						\
/*14*/	{ R13R14, R14R15, -1 },						\
/*15*/	{ R14R15, R15R16, -1 },						\
/*16*/	{ R15R16, R16R17, -1 },						\
/*17*/	{ R16R17, R17R18, -1 },						\
/*18*/	{ R17R18, R18R19, -1 },						\
/*19*/	{ R18R19, R19R20, -1 },						\
/*20*/	{ R19R20, R20R21, -1 },						\
/*21*/	{ R20R21, R21R22, -1 },						\
/*22*/	{ R21R22, R22R23, -1 },						\
/*23*/	{ R22R23, R23R24, -1 },						\
/*24*/	{ R23R24, R24R25, -1 },						\
/*25*/	{ R24R25, -1 },							\
/*26*/	{ RX, -1 },							\
/*27*/	{ RX, -1 },							\
/*28*/	{ RY, -1 },							\
/*29*/	{ RY, -1 },							\
/*30*/	{ RZ, -1 },							\
/*31*/	{ RZ, -1 },							\
/*32*/	{ R26, R27, -1 },						\
/*33*/	{ R28, R29, -1 },						\
/*34*/	{ R30, R31, -1 },						\
/*35*/	{ R0, R1, R1R2, -1 },						\
/*36*/	{ R1, R2, R0R1, R2R3, -1 },					\
/*37*/	{ R2, R3, R1R2, R3R4, -1 },					\
/*38*/	{ R3, R4, R2R3, R4R5, -1 },					\
/*39*/	{ R4, R5, R3R4, R5R6, -1 },					\
/*40*/	{ R5, R6, R4R5, R6R7, -1 },					\
/*41*/	{ R6, R7, R5R6, R7R8, -1 },					\
/*42*/	{ R7, R8, R6R7, R8R9, -1 },					\
/*43*/	{ R8, R9, R7R8, R9R10, -1 },					\
/*44*/	{ R9, R10, R8R9, R10R11, -1 },					\
/*45*/	{ R10, R11, R9R10, R11R12, -1 },				\
/*46*/	{ R11, R12, R10R11, R12R13, -1 },				\
/*47*/	{ R12, R13, R11R12, R13R14, -1 },				\
/*48*/	{ R13, R14, R12R13, R14R15, -1 },				\
/*49*/	{ R14, R15, R13R14, R15R16, -1 },				\
/*50*/	{ R15, R16, R14R15, R16R17, -1 },				\
/*51*/	{ R16, R17, R15R16, R17R18, -1 },				\
/*52*/	{ R17, R18, R16R17, R18R19, -1 },				\
/*53*/	{ R18, R19, R17R18, R19R20, -1 },				\
/*54*/	{ R19, R20, R18R19, R20R21, -1 },				\
/*55*/	{ R20, R21, R19R20, R21R22, -1 },				\
/*56*/	{ R21, R22, R20R21, R22R23, -1 },				\
/*57*/	{ R22, R23, R21R22, R23R24, -1 },				\
/*58*/	{ R23, R24, R22R23, R24R25, -1 },				\
/*59*/	{ R24, R25, R23R24, -1 }

#define RETREG(x) ((x) == SZLONGLONG || (x) == SZLDOUBLE ? R18 :	\
		(x) == SZLONG || (x) == SZFLOAT || (x) == SZDOUBLE ? R22 : \
		(x) == SZINT || (x) == SZSHORT || ISPTR(x) ? R24R25 : R24)

#define RTOLBYTES		/* bytes are numbered right to left */
#undef	FIELDOPS		/* no bit-field instructions */

#undef	TARGET_STDARGS

/* XXX - to die */
#define FPREG   RY	/* frame pointer */

/* Return a register class based on the type of the node */
#define PCLASS(p)       ((p)->n_type == INT || (p)->n_type == SHORT ? SBREG : SAREG)

int COLORMAP(int c, int *r);

#define PERMTYPE(x)	CHAR
#define GCLASS(x)	(x >= R0R1 ? CLASSB : CLASSA )
#define DECRA(x,y)	(((x) >> (y*6)) & 63)	/* decode encoded regs */
#define ENCRD(x)	(x)		/* Encode dest reg in n_reg */
#define ENCRA1(x)	((x) << 6)	/* A1 */
#define ENCRA2(x)	((x) << 12)	/* A2 */
#define ENCRA(x,y)	((x) << (6+y*6))	/* encode regs in int */

#define COM	"\t; "

#define IMMEDIATE_VALUE_MAX 63
