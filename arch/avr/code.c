/*	$Id: code.c,v 1.2 2010/01/15 15:50:36 cdidier Exp $	*/
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

#include "pass1.h"

int lastloc = -1;

/*
 * Define everything needed to print out some data (or text).
 * This means segment, alignment, visibility, etc.
 */
void
defloc(struct symtab *sp)
{
	extern char *nextsect;
	static char *loctbl[] = { "text", "data", "section .rodata" };
	TWORD t;
	char *name;
	int s;

	if (sp == NULL) {
		lastloc = -1;
		return;
	}
	t = sp->stype;
	s = ISFTN(t) ? PROG : ISCON(cqual(t, sp->squal)) ? RDATA : DATA;
	if (nextsect) {
		printf("\t.section %s\n", nextsect);
		nextsect = NULL;
		s = -1;
	} else if (s != lastloc)
		printf("\t.%s\n", loctbl[s]);
	lastloc = s;
	while (ISARY(t))
		t = DECREF(t);
	name = sp->soname ? sp->soname : exname(sp->sname);
	if (sp->sclass == EXTDEF)
		printf("\t.globl %s\n", name);
	if (ISFTN(t))
		printf("\t.type %s,#function\n", name);
	if (sp->slevel == 0) {
		printf("%s:\n", name);
	} else {
		printf(LABFMT ":\n", sp->soffset);
	}
}

/*
 * End-of-function: deals with struct return here.
 */
void
efcode()
{
	if (cftnsp->stype != STRTY+FTN && cftnsp->stype != UNIONTY+FTN)
		return;
	/* XXX */
	cerror("efcode");
}

extern int argsiz(TWORD t);
/*
 * Beginning-of-function
 * 'sp' is an array of indices in symtab for the arguments.
 * 'cnt' is the number of arguments.
 */
void
bfcode(struct symtab **sp, int cnt)
{
	struct symtab *sp2;
	int i;
	NODE *n, *q;
	int offset = 0;

	if (cftnsp->stype == STRTY+FTN || cftnsp->stype == UNIONTY+FTN) {	
		uerror("no struct return yet");
		/* Function returns struct, adjust arg offset */
		for (i = 0; i < cnt; i++)
			sp[i]->soffset += SZPOINT(INT);
	}

	/* put arguments in temporaries */
#if 0
	for (n = 0, i = 0; i < cnt; i++) {
		s = sp[i];
		if (s->stype == STRTY || s->stype == UNIONTY ||
		    cisreg(s->stype) == 0)
			continue;
		sz = tsize(s->stype, s->sdf, s->ssue);
		switch (sz) {
			/* XXX */
		default:
			break;
		}
		s->soffset = offset;
		offset += sz;
	}
#endif
	for (i = 0; i < cnt; i++) {
		if (sp[i]->stype == STRTY || sp[i]->stype == UNIONTY ||
		    cisreg(sp[i]->stype) == 0)
			continue;
		if (cqual(sp[i]->stype, sp[i]->squal) & VOL)
			continue;
		sp2 = sp[i];
		n = tempnode(0, sp[i]->stype, sp[i]->sdf, sp[i]->ssue);

		q = block(OREG, NIL, NIL, sp2->stype, sp2->sdf, sp2->ssue);
		q->n_rval = RY;
		/* XXX How to calculate offset ? */
		q->n_lval = sp2->soffset/SZCHAR + offset;

		n = buildtree(ASSIGN, n, q);
		sp[i]->soffset = regno(n->n_left);
		sp[i]->sflags |= STNODE;
		ecomp(n);
	}
}

/*
 * Beginning-of-job (before compilation starts): initialises data
 * structures specific for the local machine.
 */
void
bjobcode(void)
{ 
	printf("__SP_H__ = 0x3e\n"
	    "__SP_L__ = 0x3d\n");

	/* XXX varargs */
}

/*
 * Compute the alignment of object with type 't'.
 */
int
fldal(unsigned int t)
{
	/* XXX */
	uerror("illegal field type");
	return(ALINT);
}

/*
 * Beginning-of-code: finished generating function prologue
 *
 * by now, the automatics and register variables are allocated
 */
void
bccode()
{
	/* XXX */
	SETOFF(autooff, SZINT);
}

/*
 * End-of-job (called just before final exit)
 * 'flag' is 1 if errors, 0 if none.
 */
void
ejobcode(int flag)
{
#define _MKSTR(x) #x
#define MKSTR(x) _MKSTR(x)
#define OS MKSTR(TARGOS)
	printf("	.ident \"PCC: %s (%s)\"\n", PACKAGE_STRING, OS);
}

/*
 * Called with a function call with arguments as argument.
 * This is done early in buildtree() and only done once.
 */
NODE *
funcode(NODE *p)
{
	NODE *r, *l;

	/* Fix function call arguments. Just add funarg */
	for (r = p->n_right; r->n_op == CM; r = r->n_left) {
		if (r->n_right->n_op != STARG)
			r->n_right = block(FUNARG, r->n_right, NIL,
			    r->n_right->n_type, r->n_right->n_df,
			    r->n_right->n_sue);
	}
	if (r->n_op != STARG) {
		l = talloc();
		*l = *r;
		r->n_op = FUNARG;
		r->n_left = l;
		r->n_type = l->n_type;
	}

	return p;
}

/*
 * fix up type of field p
 */
void
fldty(struct symtab *p)
{
}

/* 
 * Build target-dependent switch tree/table.
 *
 * Return 1 if successfull, otherwise return 0 and the
 * target-independent tree will be used.
*/
int
mygenswitch(int num, TWORD type, struct swents **p, int n)
{
	return 0;
}
