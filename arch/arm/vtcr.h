#ifndef __VTCR_H__
#define __VTCR_H__

/* VTCR ATTRIBUTES */
#define VTCR_SL0_SECOND_LEVEL       0x0
#define VTCR_SL0_FIRST_LEVEL        0x1
#define VTCR_SL0_BIT                6

/* ORGN0 & IRGN0 bit are the same */
#define VTCR_NONCACHEABLE              0X0
#define VTCR_WRITEBACK_CACHEABLE           0x1
#define VTCR_WRITE_THROUGH_CACHEABLE             0x2
#define VTCR_WRITEBACK_NO_CACHEABLE           0x3
#define VTCR_ORGN0_BIT              10
#define VTCR_IRGN0_BIT              8
#define MEM_ATTR_MASK   0x0F

#endif // __VTCR_H__
