#ifndef __ARMV7_TLB_H__
#define __ARMV7_TLB_H__


#ifdef CONFIG_C99
#include "c99.h"
#endif

// TODO: add tlb-related operations
/* Invalidate entire unified TLB */
#define invalidate_unified_tlb(val)      asm volatile(\
                " mcr     p15, 0, %0, c8, c7, 0\n\t" \
                : : "r" ((val)) : "memory", "cc")

#endif /* tlb.h */
