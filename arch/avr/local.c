/*	$Id: local.c,v 1.1 2010/01/15 15:19:51 cdidier Exp $	*/
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

/*
 * Do local transformations on an expression tree preparitory to its
 * being written out in intermediate code.
 */
NODE *
clocal(NODE *p)
{
	struct symtab *q;
	NODE *r, *l;
	int o, m;

	switch (o = p->n_op) {
	case NAME:
		if ((q = p->n_sp) == NULL)
			return p;
		if (blevel == 0)
			return p;
		switch (q->sclass) {
		case PARAM:
		case AUTO:
			/* fake up a structure reference */
			r = block(REG, NIL, NIL, PTR+STRTY, 0, 0);
			r->n_lval = 0;
			r->n_rval = FPREG;
			p = stref(block(STREF, r, p, 0, 0, 0));
			break;
		case REGISTER:
			p->n_op = REG;
			p->n_lval = 0;
			p->n_rval = q->soffset;
			break;
		case STATIC:
			if (q->slevel > 0) {
				p->n_lval = 0;
				p->n_sp = q;
			}
		default:
			break;
		}
		break;

	case FORCE:
		/* put return value in return reg */
		p->n_op = ASSIGN;
		p->n_right = p->n_left;
		p->n_left = block(REG, NIL, NIL, p->n_type, 0, MKSUE(INT));
		p->n_left->n_rval = RETREG(p->n_type);
		break;

	case PCONV:
		/* Remove redundant PCONV's. Be careful */
		l = p->n_left;
		if (l->n_op == ICON) {
			l->n_lval = (unsigned)l->n_lval;
			goto delp;
		}
		if (l->n_type < INT || l->n_type == LONGLONG ||
		    l->n_type == ULONGLONG) {
			/* float etc? */
			p->n_left = block(SCONV, l, NIL,
			    UNSIGNED, 0, MKSUE(UNSIGNED));
			break;
		}
		/* if left is SCONV, cannot remove */
		if (l->n_op == SCONV)
			break;

		/* avoid ADDROF TEMP */
		if (l->n_op == ADDROF && l->n_left->n_op == TEMP)
			break;

		/* if conversion to another pointer type, just remove */
		if (p->n_type > BTMASK && l->n_type > BTMASK)
			goto delp;
		break;

	delp:   l->n_type = p->n_type;
		l->n_qual = p->n_qual;
		l->n_df = p->n_df;
		l->n_sue = p->n_sue;
		nfree(p);
		p = l;
		break;

	case SCONV:
		l = p->n_left;
		o = l->n_op;
		m = p->n_type;
		if (o == ICON) {
			CONSZ val = l->n_lval;

			if (!ISPTR(m)) /* Pointers don't need to be conv'd */
				switch (m) {
			case BOOL:
				l->n_lval = l->n_lval != 0;
				break;
			case CHAR:
				l->n_lval = (char)val;
				break;
			case SHORT:
			case INT:
				l->n_lval = (short)val;
				break;
			case VOID:
				break;
			default:
				cerror("unknown type %d", m);
			}
			l->n_type = m;
			l->n_sue = MKSUE(m);
			nfree(p);
			return l;
		} else {
			/* no need to convert CHAR ops to INT, XXX fix this */
			if (l->n_type == CHAR && p->n_type == INT) {
				nfree(p);
				return l;
			} else if (m == CHAR
			    && l->n_left->n_type == CHAR
			    && l->n_right->n_type == CHAR) {
				l->n_type = CHAR;
				nfree(p);
				return l;
			}
			cerror("not handled SCONV");
		}
	}
	return p;
}

/*
 * Called before sending the tree to the backend.
 */
void
myp2tree(NODE *p)
{
	/* XXX */
}

/*    
 * Called during the first pass to determine if a NAME can be addressed.
 *  
 * Return nonzero if supported, otherwise return 0.
 */
int
andable(NODE *p)
{
	return 1; /* XXX verify */
}

/*
 * Called just after function arguments are built.  Re-initialize the
 * offset of the arguments on the stack.
 */
void
cendarg()
{
	autooff = AUTOINIT;
}

/*
 * Return 1 if a variable of type type is OK to put in register.
 */
int
cisreg(TWORD t)
{
	if (t == FLOAT || t == DOUBLE || t == LDOUBLE ||
	    t == LONGLONG || t == ULONGLONG)
		return 0; /* not yet */
	return 1;
}

/*
 * Used for generating pointer offsets into structures and arrays.
 *
 * Return a node, for structure references, which is suitable for
 * being added to a pointer of type 't', in order to be off bits offset
 * into a structure
 *
 * 't', 'd', and 's' are the type, dimension offset, and sizeoffset.
 *
 * Be careful about only handling first-level pointers, the following
 * indirections must be fullword.
 */
NODE *
offcon(OFFSZ off, TWORD t, union dimfun *d, struct suedef *sue)
{
	if (xdebug)
		printf("offcon: OFFSZ %lld type %x dim %p siz %d\n",
		    off, t, d, sue->suesize);

	return bcon(off/SZCHAR); /* Default */
}

/*
 * Allocate bits from the stack.
 *
 * 'p' is the tree which represents the type being allocated.
 * 'off' is the number of 'p's to be allocated.
 * 't' is the storeable node where the address is written.
 */
void
spalloc(NODE *t, NODE *p, OFFSZ off)
{
	NODE *sp;

	/* XXX TODO n_rval = Stack Pointer (indirect) */
	cerror("spalloc");

	sp = buildtree(MUL, p, bcon(off/SZCHAR)); /* XXX word alignment? */

	/* sub the size from sp */
	sp = block(REG, NIL, NIL, p->n_type, 0, MKSUE(INT));
	sp->n_lval = 0;
	sp->n_rval = 0;
	ecomp(buildtree(MINUSEQ, sp, p));

	/* save the address of sp */
	sp = block(REG, NIL, NIL, PTR+INT, t->n_df, t->n_sue);
	sp->n_lval = 0;
	sp->n_rval = 0;
	t->n_type = sp->n_type;
	ecomp(buildtree(ASSIGN, t, sp)); /* Emit! */
}

/*
 * Print out a string of characters.
 * Assume that the assembler understands C-style escape
 * sequences.
 */
void
instring(struct symtab *sp)
{
	char *s, *str;

	defloc(sp);
	str = sp->sname;

	/* be kind to assemblers and avoid long strings */
	printf("	.ascii \"");
	for (s = str; *s != 0; ) {
		if (*s++ == '\\')
			esccon(&s);
		if (s - str > 60) {
			fwrite(str, 1, s - str, stdout);
			printf("\"\n\t.ascii \"");
			str = s;
		}
	}
	fwrite(str, 1, s - str, stdout);
	printf("\\0\"\n");
}

static int inbits = 0, inval = 0;

/*
 * set 'fsz' bits in sequence to zero.
 */
void 
zbits(OFFSZ off, int fsz)
{
	int m;

	if (idebug)
		printf("zbits off %lld, fsz %d inbits %d\n", off, fsz, inbits);
	if ((m = (inbits % SZCHAR))) {
		m = SZCHAR - m;
		if (fsz < m) {
			inbits += fsz;
			return;
		} else {
			fsz -= m;
			printf("\t.byte %d\n", inval);
			inval = inbits = 0;
		}
	}
	if (fsz >= SZCHAR) {
		printf("\t.space %d\n", fsz/SZCHAR);
		fsz -= (fsz/SZCHAR) * SZCHAR;
	}
	if (fsz) {
		inval = 0;
		inbits = fsz;						 
	}
}

/*
 * Initialize a bitfield.
 */
void
infld(CONSZ off, int fsz, CONSZ val)
{
	if (idebug)
		printf("infld off %lld, fsz %d, val %lld inbits %d\n",
		    off, fsz, val, inbits);
	val &= ((CONSZ)1 << fsz)-1;
	while (fsz + inbits >= SZCHAR) {
		inval |= (val << inbits);
		printf("\t.byte %d\n", inval & 255);
		fsz -= (SZCHAR - inbits);
		val >>= (SZCHAR - inbits);
		inval = inbits = 0;
	}
	if (fsz) {
		inval |= (val << inbits);
		inbits += fsz;
	}
}

/*
 * Print an integer constant node, may be associated with a label.
 * Do not free the node after use.
 *
 * 'off' is bit offset from the beginning of the aggregate
 * 'fsz' is the number of bits this is referring to
 */
void
ninval(CONSZ off, int fsz, NODE *p)
{
	cerror("ninval"); /* XXX */
}

/*
 * Make a name look like an external name in the local machine.
 */
char *
exname(char *p)
{
	/* XXX verify */
	return p == NULL ? "" : p;
}

/*
 * Map types which are not defined on the local machine.
 */
TWORD
ctype(TWORD type)
{
/*	cerror("ctype"); XXX */

	switch (BTYPE(type)) {
	}
	return type;
}

/*
 * Before calling a function do any tree re-writing for the local machine.
 * 
 * 'p' is the function tree (NAME)
 * 'q' is the CM-separated list of arguments.
 */
void
calldec(NODE *p, NODE *q)
{
}

/*
 * While handling uninitialised variables, handle variables marked extern.
 */
void
extdec(struct symtab *q)
{
}

/*
 * Make a common declaration for id, if reasonable.
 */
void
defzero(struct symtab *sp)
{
	cerror("rdefzero"); /* XXX */
}

char *nextsect;

/*
 * Give target the opportunity of handling pragmas.  
 */
int
mypragma(char **ary)
{
	/* XXX maybe __progmem */
	return 0;
}

/*
 * Called when a identifier has been declared.
 */
void
fixdef(struct symtab *sp)
{
	/* XXX maybe */
}

void
pass1_lastchance(struct interpass *ip)
{
}
