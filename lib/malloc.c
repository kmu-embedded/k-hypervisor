/* $Id: malloc.c,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $ */

/*
 * Copyright (c) 2000-2002 Opsycon AB  (www.opsycon.se)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <lib/malloc.h>
#include <lib/null.h>
#include <lib/print.h>

// TODO(wonseok): need a mutex in malloc and free.
HEADER   base;
HEADER  *freep = NULL;

void *malloc(size_t nbytes)
{
	HEADER *p, *q;	/* K&R called q, prevp */
	unsigned nunits;

	nunits = (nbytes + sizeof (HEADER) - 1) / sizeof (HEADER) + 1;
	if ((q = freep) == NULL) {	/* no free list yet */
		base.s.ptr = freep = q = &base;
		base.s.size = 0;
	}
	for (p = q->s.ptr;; q = p, p = p->s.ptr) {
		if (p->s.size >= nunits) {	/* big enough */
			if (p->s.size == nunits)	/* exactly */
				q->s.ptr = p->s.ptr;
			else {		/* allocate tail end */
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = q;
			return ((char *)(p + 1));
		}
		if (p == freep)
			if ((p = morecore (nunits)) == NULL) {
				return (NULL);
			}
	}
}

void free(void *ap)
{
	HEADER *p, *q;

	p = (HEADER *) ap - 1;
	for (q = freep; !(p > q && p < q->s.ptr); q = q->s.ptr)
		if (q >= q->s.ptr && (p > q || p < q->s.ptr))
			break;
	if (p + p->s.size == q->s.ptr) {
		p->s.size += q->s.ptr->s.size;
		p->s.ptr = q->s.ptr->s.ptr;
	} else
		p->s.ptr = q->s.ptr;
	if (q + q->s.size == p) {
		q->s.size += p->s.size;
		q->s.ptr = p->s.ptr;
	} else
		q->s.ptr = p;
	freep = q;
}

