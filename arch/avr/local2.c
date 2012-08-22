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

#include "pass2.h"

char *rnames[] = {
	"r0", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11",
	"r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19",
	"r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27",
	"r28", "r29", "r30", "r31",
	"X", "Y", "Z"
};

void
deflab(int label)
{
	printf(LABFMT ":\n", label);
}

/*
 * calculate stack size and offsets
 */
static int
offcalc(struct interpass_prolog *ipp)
{
	int addto;

	addto = p2maxautooff;
	addto -= AUTOINIT / SZCHAR;
	return addto;
}


void
prologue(struct interpass_prolog *ipp)
{
	int i, addto;

	fputs("\t/* prologue */\n", stdout);
	/* save R28:R29 (frame pointer) to the stack */
	printf("\tpush %s\n"
	    "\tpush %s\n", rnames[R28], rnames[R29]);
	/* save Call-saved registers to the stack */
	for (i = R2; i < R17; i++)
		if (TESTBIT(ipp->ipp_regs, i))
			printf("\tpush %s\n", rnames[i]);
	/* set the frame pointer */
	printf("\tin %s,__SP_L__\n"
	    "\tin %s,__SP_H__\n", rnames[R28], rnames[R29]);
	/* alloc space on stack */
	addto = offcalc(ipp);
	if (addto != 0) {
		do {
			int val = addto > IMMEDIATE_VALUE_MAX ?
			    IMMEDIATE_VALUE_MAX : addto;
			printf("\tsbiw %s,%d\n", rnames[R28], val);
			addto -= val;
		} while (addto > 0);
		printf("\tout __SP_L__,%s\n"
		    "\tout __SP_H__,%s\n", rnames[R28], rnames[R29]);
	}
	fputs("\t/* prologue end */\n", stdout);
}

void
eoftn(struct interpass_prolog *ipp)
{
	int i, addto;

	fputs("\t/* epilogue */\n", stdout);
	addto = offcalc(ipp);
	if (addto != 0) {
		do {
			int val = addto > IMMEDIATE_VALUE_MAX ?
			    IMMEDIATE_VALUE_MAX : addto;
			printf("\tadiw %s,%d\n", rnames[R28], val);
			addto -= val;
		} while (addto > 0);
		printf("\tout __SP_L__,%s\n"
		    "\tout __SP_H__,%s\n", rnames[R28], rnames[R29]);
	}
	/* restore Call-saved registers from the stack */
	for (i = R17; i >= R2; i--)
		if (TESTBIT(ipp->ipp_regs, i))
			printf("\tpop %s\n", rnames[i]);
	/* dealloc space on stack */
	/* restore R28:R29 (frame pointer) from the stack */
	printf("\tpop %s\n"
	    "\tpop %s\n"
	    "\tret\n"
	    "\t/* epilogue end */\n", rnames[R29], rnames[R28]);
}

/*
 * add/sub/...
 */
void
hopcode(int f, int o)
{
	char *str;

	switch (o) {
	case PLUS:
		str = "add";
		break;
	case MINUS:
		str = "sub";
		break;
	case AND:
		str = "and";
		break;
	case OR:
		str = "or";
		break;
	case ER:
		str = "eor";
		break;
	default:
		comperr("hopcode2: %d", o);
		str = NULL;
	}
	printf("%s%c", str, f);
}

int
fldexpand(NODE *p, int cookie, char **cp)
{
	comperr("fldexpand"); /* XXX */
	return 1;
}

/*
 * Handle the Z directives in the table.
 */
void
zzzcode(NODE *p, int c)
{
	NODE *l;
	int sz;

	switch (c) {
	/* store to the stack or to an address in register */
	case 'T':
		l = p->n_left;
		sz = szty(p->n_type);
		if (l->n_op == OREG && !ISPTR(l->n_type) && l->n_rval != RY) {
			if (sz == 1)
				expand(p, 0,
				    "	movw r30,AL"
				    COM "assign AR to the address in AL+1:AL\n"
				    "	st ZZ,AR\n");
			else if (sz == 2)
				expand(p, 0,
				    "	movw r30,AL"
				    COM "assign UR:AR to the address in AL+1:AL\n"
				    "	st ZZ+,AR\n"
				    "	st ZZ,UR\n");
			else
				comperr("cannot assign to an address in reg");
		} else {
			if (sz == 1)
				expand(p, 0,
				    "	ZS AL,AR" COM "assign AR to AL\n");
			else if (sz == 2)
				expand(p, 0,
				    "	ZS AL,AR" COM "assign UR:AR to Z1:AL\n"
				    "	std Z1,UR\n");
			else
				comperr("cannot assign to the stack");
		}
		return;

	/* choose between "st" and "std" */
	case 'S':
		fputs(p->n_left->n_lval == 0 ? "st" : "std", stdout);
		return;

	/* choose between "ld" and "ldd" */
	case 'L':
		if (p->n_op == OREG)
			fputs(p->n_lval == 0 ? "ld" : "ldd", stdout);
		else if (p->n_right->n_op == OREG)
			fputs(p->n_right->n_lval == 0 ? "ld" : "ldd", stdout);
		return;

	/* print upper parts of an OREG */
	case '1':
		if (p->n_op == OREG)
			upput(p, SZCHAR);
		else if (p->n_left->n_op == OREG)
			upput(p->n_left, SZCHAR);
		else if (p->n_right->n_op == OREG)
			upput(p->n_right, SZCHAR);
		return;

	case 'Z':
		printf("%s", rnames[RZ]);
		return;

	default:
		comperr("bad zzzcode %c", c);
	}
}

/* ARGSUSED */
int
rewfld(NODE *p)
{
	return 1;
}

int
flshape(NODE *p)
{
	comperr("flshape"); /* XXX */

	return SRREG; /* put it into a register */
}

/* INTEMP shapes must not contain any temporary registers */
/* XXX should this go away now? */
int
shtemp(NODE *p)
{
	return 0;
}

void
adrcon(CONSZ val)
{
	comperr("adrcon"); /* XXX */

	printf(CONFMT, val);
}

void
conput(FILE *io, NODE *p)
{
	switch (p->n_op) {
	case ICON:
		if (p->n_name[0] != '\0') {
			fputs(p->n_name, io);
			if (p->n_lval != 0)
				fprintf(io, "+" CONFMT, p->n_lval);
		} else
			fprintf(io, CONFMT, p->n_lval);
		return;

	default:
		comperr("illegal conput, p %p", p);
	}
}

/*ARGSUSED*/
void
insput(NODE *p)
{
	comperr("insput");
}

/*
 * Write out the upper address, like the upper register of a 2-register
 * reference, or the next memory location.
 */
void
upput(NODE *p, int size)
{
	int rval;

	size /= SZCHAR;
	switch (p->n_op) {
	case REG:
		rval = p->n_rval >= R0R1 ? p->n_rval - R0R1 : p->n_rval;
		fputs(rnames[rval+1], stdout);
		break;

	case NAME:
	case OREG:
		p->n_lval += size;
		adrput(stdout, p);
		p->n_lval -= size;
		break;

	case ICON:
		fprintf(stdout, CONFMT, p->n_lval >> 8);
		break;

	default:
		comperr("upput bad op %d size %d", p->n_op, size);
	}
}

/*
 * Output an address, with offsets, from 'p'.
 */
void
adrput(FILE *io, NODE *p)
{
	int rval;

	if (p->n_op == FLD)
		p = p->n_left;
	switch (p->n_op) {
	case NAME:
		if (p->n_name[0] != '\0') {
			fputs(p->n_name, io);
			if (p->n_lval != 0)
				fprintf(io, "+" CONFMT, p->n_lval);
		} else
			fprintf(io, CONFMT, p->n_lval);
		return;

	case OREG:
		rval = p->n_rval >= R0R1 ? p->n_rval - R0R1 : p->n_rval;
		fprintf(io, "%s", rnames[rval]);
		if (p->n_lval != 0)
			fprintf(io, "+" CONFMT, p->n_lval);
		return;

	case ICON:
		/* addressable value of the constant */
		conput(io, p);
		return;

	case REG:
		rval = p->n_rval >= R0R1 ? p->n_rval - R0R1 : p->n_rval;
		fputs(rnames[rval], io);
		return;

	default:
		comperr("illegal address, op %d, node %p", p->n_op, p);
		return;
	}
}

/*
 * these mnemonics match the order of the preprocessor decls
 * EQ, NE, LE, LT, GE, GT, ULE, ULT, UGE, UGT
 */

static char *
ccbranches[] = {
	"breq",	  /* branch if equal */
	"brne",	  /* branch if not-equal */
	"XXX",	  /* branch if less-than-or-equal */
	"brlt",	  /* branch if less-than */
	"brge",	  /* branch if greater-than-or-equal */
	"XXX",	  /* branch if greater-than */
	/* what should these be ? */
	"XXX",	  /* branch if lower-than-or-same */
	"brlo",	  /* branch if lower-than */
	"brsh",	  /* branch if higher-than-or-same */
	"XXX",	  /* branch if higher-than */
};

static int tmpbrlab = 0;

/*
 * Print conditional and unconditional branches.
 */
void
cbgen(int o, int lab)
{
	if (o < EQ || o > UGT)
		comperr("bad conditional branch: %s", opst[o]);

	if (o == LE) {
		printf("\t%s " LABFMT COM "conditional branch\n"
		    "\t%s " LABFMT "\n",
		    ccbranches[EQ-EQ], lab, ccbranches[LT-EQ], lab);
	} else if (o == GT) {
		printf("\t%s .tmpbrlab%d" COM "conditional branch\n"
		    "\t%s " LABFMT "\n"
		    ".tmpbrlab%d\n",
		    ccbranches[EQ-EQ], tmpbrlab, ccbranches[GE-EQ], lab, tmpbrlab);
		++tmpbrlab;
	} else if (o == ULE) {
		printf("\t%s " LABFMT COM "conditional branch\n"
		    "\t%s " LABFMT "\n",
		    ccbranches[EQ-EQ], lab, ccbranches[ULT-EQ], lab);
	} else if (o == UGT) {
		printf("\t%s .tmpbrlab%d" COM "conditional branch\n"
		    "\t%s " LABFMT "\n"
		    ".tmpbrlab%d\n",
		    ccbranches[EQ-EQ], tmpbrlab, ccbranches[UGE-EQ], lab, tmpbrlab);
		++tmpbrlab;
	} else {
		printf("\t%s " LABFMT COM "conditional branch\n",
		    ccbranches[o-EQ], lab);
	}
}

void
myreader(struct interpass *ipole)
{
	/* XXX */
}

/*
 * Remove some PCONVs after OREGs are created.
 */
static void
pconv2(NODE *p, void *arg)
{
	NODE *q;

	if (p->n_op == PLUS) {
		if (p->n_type == (PTR+SHORT) || p->n_type == (PTR+USHORT)) {
			if (p->n_right->n_op != ICON)
				return;
			if (p->n_left->n_op != PCONV)
				return;
			if (p->n_left->n_left->n_op != OREG)
				return;
			q = p->n_left->n_left;
			nfree(p->n_left);
			p->n_left = q;
			/*
			 * This will be converted to another OREG later.
			 */
		}
	}
}

void
mycanon(NODE *p)   
{
	walkf(p, pconv2, 0);
}

void
myoptim(struct interpass *ip)
{
}

/*
 * Register move: move contents of register 's' to register 'r'.
 */
void
rmove(int s, int d, TWORD t)
{
	comperr("rmove"); /* XXX */
}

/*
 * For class 'c', find worst-case displacement of the number of
 * registers in the array 'r' indexed by class.
 */
int
COLORMAP(int c, int *r)
{
	int num = 0;

	num += r[CLASSA];
	num += 2*r[CLASSB];
	switch (c) {
	case CLASSA:
		return num < MAXREGSA;
	case CLASSB:
		return num < MAXREGSB;
	}
	cerror("colormap");
	return 0; /* XXX gcc */
}

/*
 * Return a class suitable for a specific type.
 */
int
gclass(TWORD t)
{
	int sz = szty(t);

	if (sz == 2)
		return CLASSB;
	return CLASSA;
}

static int
argsiz(TWORD t)
{
	/* struct args are on stack */
	if (t == STRTY || t == UNIONTY)
		return 0;
	return szty(t);
}

/*
 * Calculate argument sizes.
 */
void
lastcall(NODE *p)
{
	NODE *op = p;
	int size = 0;

	p->n_qual = 0;
	if (p->n_op != CALL && p->n_op != FORTCALL && p->n_op != STCALL)
		return;
	for (p = p->n_right; p->n_op == CM; p = p->n_left)
		size += argsiz(p->n_right->n_type);
	size += argsiz(p->n_type);
	if (size)
		printf("	/* argument sizes: %d\n", size);
	op->n_qual = size;
}

/*
 * Special shapes.
 */
int
special(NODE *p, int shape)
{
	comperr("special"); /* XXX */
	return SRNOPE;
}

/*
 * Target-dependent command-line options.
 */
void
mflags(char *str)
{
	cerror("mflags"); /* XXX */
}

/*
 * Do something target-dependent for xasm arguments.
 */
int
myxasm(struct interpass *ip, NODE *p)
{
	return 0;
}
