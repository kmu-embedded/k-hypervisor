#ifndef __GENERIC_TIMER_H__
#define __GENERIC_TIMER_H__

#include <stdint.h>
#include <asm/asm.h>

#define NS_PL2_PTIMER_IRQ   26
#define NS_VTIMER_IRQ       27
#define PL1_PTIMER_IRQ      29
#define NS_PL1_PTIMER_IRQ   30

/* *** from sys/arch/arm/include/asm.h of NetBSD *** */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *  from: @(#)asm.h 5.5 (Berkeley) 5/7/91
 */

#define __BIT(n)    	(1 << (n))
#define __BITS(hi,lo)   ((~((~0)<<((hi)+1)))&((~0)<<(lo)))

/* *** from sys/arch/arm/include/armreg.h of NetBSD *** */
/*
 * Copyright (c) 2016 Ingu Kang
 * Copyright (c) 1998, 2001 Ben Harris
 * Copyright (c) 1994-1996 Mark Brinicombe.
 * Copyright (c) 1994 Brini.
 * All rights reserved.
 *
 * This code is derived from software written for Brini by Mark Brinicombe
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
 *  This product includes software developed by Brini.
 * 4. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BRINI ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL BRINI OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Defines for ARM Generic Timer */
#define ARM_CNTCTL_ENABLE       __BIT(0) // Timer Enabled
#define ARM_CNTCTL_IMASK        __BIT(1) // Mask Interrupt
#define ARM_CNTCTL_ISTATUS      __BIT(2) // Interrupt is pending

#define ARM_CNTKCTL_PL0PTEN     __BIT(9)
#define ARM_CNTKCTL_PL0VTEN     __BIT(8)
#define ARM_CNTKCTL_EVNTI       __BITS(7,4)
#define ARM_CNTKCTL_EVNTDIR     __BIT(3)
#define ARM_CNTKCTL_EVNTEN      __BIT(2)
#define ARM_CNTKCTL_PL0PCTEN    __BIT(1)
#define ARM_CNTKCTL_PL0VCTEN    __BIT(0)

#define ARM_CNTHCTL_EVNTI       __BITS(7,4)
#define ARM_CNTHCTL_EVNTDIR     __BIT(3)
#define ARM_CNTHCTL_EVNTEN      __BIT(2)
#define ARM_CNTHCTL_PL1PCEN     __BIT(1)
#define ARM_CNTHCTL_PL1PCTEN    __BIT(0)

#endif /* INCLUDE_ARCH_ARM_ARMV7_GENERIC_TIMER_H_ */
