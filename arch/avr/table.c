/*	$Id: table.c,v 1.1 2010/01/15 15:19:51 cdidier Exp $	*/
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
 * A template has five logical sections:
 *
 *      1) subtree (operator); goal to achieve (cookie)
 *      2) left node descendent of operator (node class; type)
 *      3) right node descendent of operator (node class; type)
 *      4) resource requirements (number of scratch registers);
 *	 subtree rewriting rule
 *      5) emitted instructions
 */

#include "pass2.h"

#define T8	TCHAR
#define T16	TINT|TSHORT
#define TU8	TUCHAR
#define TU16	TUINT|TUSHORT

struct optab table[] = {
/* First entry must be an empty entry */
{ -1, FOREFF, SANY, TANY, SANY, TANY, 0, 0, "", },

/*
 * Conversions
 */

/* PCONVs are usually not necessary */
{ PCONV,	0,
	SANY,	T8|TPOINT,
	SANY,	T8|TPOINT,
		0,	RLEFT,
		"", },

/*
 * Subroutines calls
 */

{ UCALL,	FOREFF,
	SCON,	TANY,
	SANY,	TANY,
		0,	0,
		"	rcall AL\n", },

{ CALL,		FOREFF,
	SCON,	TANY,
	SANY,	TANY,
		0,	0,
		"	rcall AL\n", },

/*
 * Jumps
 */

{ GOTO,		FOREFF,
	SCON,   TANY,
	SANY,   TANY,
		0,	RNOP,
		"	rjmp LL" COM "jump to LL\n", },

/*
 * Operations
 */

/* 16-bit operations (with carry) */
{ PLUS,		INBREG|FOREFF,
	SBREG,	T16,
	SBREG,	T16,
		0,	RLEFT,
		"	add AL,AR" COM "add UR:AR to UL:AL\n"
		"	adc UL,UR\n", },

{ MINUS,	INBREG|FOREFF,
	SBREG,	T16,
	SBREG,	T16,
		0,	RLEFT,
		"	sub AL,AR" COM "sub UR:AR to UL:AL\n"
		"	sbc UL,UR\n", },

/* 8-bit immediates */
{ MINUS,	INBREG|FOREFF,
	SBREG,	T8,
	SCON,	T8,
		0,	RLEFT,
		"	subi AL,AR" COM "subi AR to AL\n", },

{ OR,		INBREG|FOREFF,
	SBREG,	T8,
	SCON,	T8,
		0,	RLEFT,
		"	ori AL,AR" COM "ori AR to AL\n", },

{ AND,		INBREG|FOREFF,
	SBREG,	T8,
	SCON,	T8,
		0,	RLEFT,
		"	andi AL,AR" COM "andi AR to AL\n", },

/* 16-bit immediates */
{ MINUS,	INBREG|FOREFF,
	SBREG,	T16,
	SCON,	T16,
		0,	RLEFT,
		"	subi AL,lo8(AR)" COM "subi AR to UL:AL\n"
		"	sbci UL,hi8(AR)\n", },

/* Other operations */
{ OPSIMP,	INAREG|FOREFF,
	SAREG,	T8,
	SAREG,	T8,
		0,	RLEFT,
		"	O AL,AR" COM "O AR to AL\n", },

{ OPSIMP,	INBREG|FOREFF,
	SBREG,	T16,
	SBREG,	T16,
		0,	RLEFT,
		"	O AL,AR" COM "O UR:AR to UL:AL\n"
		"	O UL,UR\n", },

/*
 * Indirection operators.
 */

{ UMUL,		INAREG,
	SANY,	TANY,
	SOREG,	T8,
		NAREG,	RESC1,
		"	movw r30,AR"
		COM "load from the address in AR+1:AR to A1\n"
		"	ld A1,ZZ\n", },

{ UMUL,		INBREG,
	SANY,	TANY,
	SOREG,	T16,
		NBREG,	RESC1,
		"	movw r30,AR"
		COM "load from the address in AR+1:AR to U1:A1\n"
		"	ld A1,ZZ+\n"
		"	ld U1,ZZ\n", },

/*
 * Assignments (8 bits)
 */

{ ASSIGN,	INAREG|FOREFF,
	SAREG,	T8,
	SAREG,	T8,
		0,	RLEFT|RDEST,
		"	mov AL,AR" COM "assign AR to AL\n", },

{ ASSIGN,	FOREFF,
	SOREG,	T8,
	SAREG,	T8,
		0,	RLEFT,
		"ZT", },

{ ASSIGN,	FOREFF,
	SBREG,	T8,
	SOREG,	T8,
		0,	RLEFT,
		"	ZS AL,AR" COM "assign AR to AL\n", },

{ ASSIGN,	FOREFF,
	SNAME,	T8,
	SAREG,	T8,
		0,	RLEFT,
		"	ldi r30,lo8(AL)" COM "assign AR to name\n"
		"	ldi r31,hi8(AL)\n"
		"	st ZZ,AR\n", },

/*
 * Assignments (16 bits)
 */

{ ASSIGN,	INBREG|FOREFF,
	SBREG,	T16|TPOINT,
	SBREG,	T16|TPOINT,
		0,	RLEFT|RDEST,
		"	movw AL,AR" COM "assign UR:AR to UL:AL\n" , },

{ ASSIGN,	FOREFF,
	SOREG,	T16|TPOINT,
	SBREG,	T16|TPOINT,
		0,	RLEFT,
		"ZT", },

{ ASSIGN,	FOREFF,
	SBREG,	T16|TPOINT,
	SOREG,	T16|TPOINT,
		0,	RLEFT,
		"	ZL AL,AR" COM "assign Z1:AR to UL:AL\n"
		"	ldd UL,Z1\n", },

{ ASSIGN,	FOREFF,
	SNAME,	T16,
	SBREG,	T16|TPOINT,
		0,	RLEFT,
		"	ldi r30,lo8(AL)" COM "assign UR:AR to name\n"
		"	ldi r31,hi8(AL)\n"
		"	st ZZ+,AR\n"
		"	st ZZ,UR\n", },

/*
 * Logical/branching operators
 */

{ OPLOG,	FORCC,
	SAREG,	T8,
	SAREG,	T8,
		0,	RESCC,
		"	cp AL,AR" COM "compare AL and AR\n", },

{ OPLOG,	FORCC,
	SAREG,	T8,
	SCON,	T8,
		0,	RESCC,
		"	cpi AL,AR" COM "compare AL and AR\n", },

{ OPLOG,	FORCC,
	SBREG,	T16,
	SBREG,	T16,
		0,	RESCC,
		"	cp AL,AR" COM "compare AL:UL and AR:UR\n"
		"	cpc UL,UR\n", },

/*
 * Convert LTYPE to reg.
 */

{ OPLTYPE,	INAREG,
	SANY,	TANY,
	SCON,	T8,
		NAREG,	RESC1,
		"	ldi A1,AR" COM "load const to A1\n", },

{ OPLTYPE,	INBREG,
	SANY,	TANY,
	SCON,	T16|TPOINT,
		NBREG,	RESC1,
		"	ldi A1,lo8(AR)" COM "load const to U1:A1\n"
		"	ldi U1,hi8(AR)\n", },

{ OPLTYPE,	INAREG,
	SANY,	TANY,
	SOREG,	T8,
		NAREG,	RESC1,
		"	ZL A1,AR" COM "load AR to A1\n", },

{ OPLTYPE,	INBREG,
	SANY,	TANY,
	SOREG,	T16|TPOINT,
		NBREG,	RESC1,
		"	ZL A1,AR" COM "load Z1:AR to U1:A1\n"
		"	ldd U1,Z1\n", },

/* 
 * Arguments to functions.
 */

{ FUNARG,	FOREFF,
	SBREG,	T8,
	SANY,	T8,
		0,	RNULL,
		"	push AL" COM "save AL\n", },

{ FUNARG,	FOREFF,
	SBREG,	T16|TPOINT,
	SANY,	T16|TPOINT,
		0,	RNULL,
		"	push AL" COM "save UL:AL\n"
		"	push UL\n", },

{ FREE, FREE,   FREE,   FREE,   FREE,   FREE,   FREE,   FREE,   "help; I'm in trouble\n" },
};
