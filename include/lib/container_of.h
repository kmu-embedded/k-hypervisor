/*	$NetBSD: libkern.h,v 1.121 2015/08/30 07:55:45 uebayasi Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)libkern.h	8.2 (Berkeley) 8/5/94
 */

#ifndef __CONTAINER_OF_H__
#define __CONTAINER_OF_H__

/*
 * Return the container of an embedded struct.  Given x = &c->f,
 * container_of(x, T, f) yields c, where T is the type of c.  Example:
 *
 *	struct foo { ... };
 *	struct bar {
 *		int b_x;
 *		struct foo b_foo;
 *		...
 *	};
 *
 *	struct bar b;
 *	struct foo *fp = b.b_foo;
 *
 * Now we can get at b from fp by:
 *
 *	struct bar *bp = container_of(fp, struct bar, b_foo);
 *
 * The 0*sizeof((PTR) - ...) causes the compiler to warn if the type of
 * *fp does not match the type of struct bar::b_foo.
 * We skip the validation for coverity runs to avoid warnings.
 */
#ifdef __COVERITY__
#define __validate_container_of(PTR, TYPE, FIELD) 0
#else
#define __validate_container_of(PTR, TYPE, FIELD)			\
    (0 * sizeof((PTR) - &((TYPE *)(((char *)(PTR)) -			\
    offsetof(TYPE, FIELD)))->FIELD))
#endif

#define	container_of(PTR, TYPE, FIELD)					\
    ((TYPE *)(((char *)(PTR)) - offsetof(TYPE, FIELD))			\
	+ __validate_container_of(PTR, TYPE, FIELD))

#endif
