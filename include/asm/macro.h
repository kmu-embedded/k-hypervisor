/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     l4/arm/asm.h
 * Description:   Assembler macros etc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: asm.h,v 1.3 2004/06/04 08:20:12 htuch Exp $
 *
 ********************************************************************/

#ifndef __MACRO_H__
#define __MACRO_H__

#ifndef __ARMCC_VERSION
/* GNU tools */
#define BEGIN_PROC(name)			\
    .global name; 				\
    .align;					\
name:

#define END_PROC(name)				\
    ;

#define END

#define LABEL(name)				\
name:

#else
/* ARM tools */
MACRO
begin_proc $name
EXPORT	$name
ALIGN
$name
MEND
#define BEGIN_PROC(name)			\
    begin_proc name

#define END_PROC(name)				\
    ;

/*
 * The ADS and RVCT cpps do different things.  ADS puts blank space at the
 * start of lines, while RVCT puts no space at all! :(
 */
#if __ARMCC_VERSION < 200000
MACRO
label $name
$name
MEND
#define LABEL(name)		label name
#else
#define LABEL(name)		name
#endif

#endif

#define SECTION(x) __attribute__((section(x)))

#endif /* __MACRO_H__ */
