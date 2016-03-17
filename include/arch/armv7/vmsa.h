#ifndef __ARMV7_MM_H__
#define __ARMV7_MM_H__

#include <stdint.h>
#include <asm/asm.h>

#define read_ttbr0()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr0(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbr1()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr1(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbcr()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbcr(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair0()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair1()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_httbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_httbr(val)    asm volatile(\
                            " mcrr     p15, 4, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define read_vtcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c1, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vtcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c1, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_vttbr()            ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_vttbr(val)    asm volatile(\
                            " mcrr     p15, 6, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define HTCR_IRGN0_BIT                  8
#define HTCR_ORGN0_BIT                  10
#define HTCR_SH0_BIT                    12

#define read_htcr()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_htcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* VTCR ATTRIBUTES */
#define VTCR_SL0_SECOND_LEVEL       0x0
#define VTCR_SL0_FIRST_LEVEL        0x1
#define VTCR_SL0_BIT                6

/* ORGN0 & IRGN0 bit are the same */
//#define VTCR_NONCACHEABLE               0X0
//#define VTCR_WRITEBACK_CACHEABLE        0x1
//#define VTCR_WRITE_THROUGH_CACHEABLE    0x2
//#define VTCR_WRITEBACK_NO_CACHEABLE     0x3

#define VTCR_ORGN0_BIT                  10
#define VTCR_IRGN0_BIT                  8

/*
 * CP15 Barrier instructions
 */
#define CP15ISB asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
#define CP15DSB asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))
#define CP15DMB asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))

#endif /* armv7-mm.h */
