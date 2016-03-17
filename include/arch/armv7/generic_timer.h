#ifndef __GENERIC_TIMER_H___
#define __GENERIC_TIMER_H___

#include <stdint.h>
#include <asm/asm.h>


/* Generic Timer */
#define read_cntfrq()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntfrq(val)       asm volatile(\
                                " mcr     p15, 0, %0, c14, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthctl()          ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthctl(val)      asm volatile(\
                                " mcr     p15, 4, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_ctl()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_ctl(val)    asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_cval()       ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cnthp_cval(val) 	asm volatile(\
                              	  " mcrr     p15, 6, %0, %1, c14\n\t" \
								  : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
								  : "memory", "cc")

#define read_cnthp_tval()       ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_tval(val)   asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntkctl()          ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntkctl(val)      asm volatile(\
                                " mcr     p15, 0, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_ctl()         ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_cval()        ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 2, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntp_cval(val)  	asm volatile(\
                              	  " mcrr     p15, 2, %0, %1, c14\n\t" \
								  : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
								  : "memory", "cc")

#define read_cntp_tval()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntpct()           ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 0, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define read_cntv_ctl()         ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntv_cval()        ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 3, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntv_cval(val)  asm volatile(\
                              " mcrr     p15, 3, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cntv_tval()        ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntvct()           ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 1, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define read_cntvoff()          ({ uint32_t v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((uint64_t)v2 << 32) + (uint64_t)v1); })

#define write_cntvoff(val)    asm volatile(\
                              " mcrr     p15, 4, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#endif /* INCLUDE_ARCH_ARM_ARMV7_GENERIC_TIMER_H_ */
