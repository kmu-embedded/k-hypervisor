#ifndef __ARMV7_TLB_H__
#define __ARMV7_TLB_H__

#include <stdint.h>
#include <asm/asm.h>

// TODO: add tlb-related operations
/* Invalidate entire unified TLB */
#define invalidate_unified_tlb(val)      asm volatile(\
                " mcr     p15, 0, %0, c8, c7, 0\n\t" \
                : : "r" ((val)) : "memory", "cc")

#endif /* tlb.h */
