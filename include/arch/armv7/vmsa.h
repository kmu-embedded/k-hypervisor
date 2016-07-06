#ifndef __ARMV7_MM_H__
#define __ARMV7_MM_H__

#define HTCR_IRGN0_BIT                  8
#define HTCR_ORGN0_BIT                  10
#define HTCR_SH0_BIT                    12

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
//#define CP15ISB asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
//#define CP15DSB asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))
//#define CP15DMB asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))

#endif /* armv7-mm.h */
