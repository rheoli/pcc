/*	$Id: order.c,v 1.1 2010/01/15 15:19:51 cdidier Exp $	*/
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

/*
 * Is it legal to make an OREG or NAME entry which has an
 * offset of 'off', (from a register of 'r'), if the
 * resulting thing had type 't'?
 */
int
notoff(TWORD t, int r, CONSZ off, char *cp)
{
	/* An offset cannot be greater than 63. */
	if (off+szty(t) > IMMEDIATE_VALUE_MAX+1)
		return 1; /* NO */
	return 0;  /* YES */
}

/*
 * Turn a UMUL-referenced node into OREG.
 * Be careful about register classes, this is a place where classes change.
 */
void
offstar(NODE *p, int shape)
{
	if (x2debug)
		printf("offstar(%p)\n", p);

	if (isreg(p))
		return; /* Is already OREG */
	geninsn(p, INBREG);
}


/*
 * Do the actual conversion of offstar-found OREGs into real OREGs.
 */
void
myormake(NODE *p)
{
	if (x2debug)
		printf("myormake(%p)\n", p);
}

/*
 * Shape matches for UMUL.  Cooperates with offstar().
 */
int
shumul(NODE *p, int shape)
{
	if (x2debug)
		printf("shumul(%p)\n", p);

	if (shape & SOREG)
		return SROREG;
	return SRNOPE;
}

/*
 * Rewrite operations on binary operators (like +, -, etc...).
 * Called as a result of a failed table lookup.
 *
 * Return nonzero to retry table search on new tree, or zero to fail.
 */
int
setbin(NODE *p)
{
	return 0;
}

/*
 * Setup for assignment operator.
 */
int
setasg(NODE *p, int cookie)
{
	return 0;
}

/*
 * Setup for unary operator.
 */
int
setuni(NODE *p, int cookie)
{
	return 0;
}

/*
 * Special handling of some instruction register allocation.
 *
 * Called as a result of specifying NSPECIAL in the table.
 */
struct rspecial *
nspecial(struct optab *q)
{
	comperr("nspecial"); /* XXX */

	switch (q->op) {
	default:
		break;
	}
	return 0; /* XXX gcc */
}

/*
 * Set evaluation order of a binary node if it differs from default.
 */
int
setorder(NODE *p)
{
	return 0;
}

/*
 * Set registers "live" at function calls (like arguments in registers).
 * This is for liveness analysis of registers.
 */
int *
livecall(NODE *p)
{
	static int r[] = { -1 }; /* Terminate with -1 */

	return r;
}

/*
 * Signal whether the instruction is acceptable for this target.
 */
int
acceptable(struct optab *op)
{
	return 1;
}
