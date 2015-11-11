#ifndef __ARMV7_P15_H__
#define __ARMV7_P15_H__

/* Generic ARM Registers */
#define read_cpsr()             ({ unsigned int rval; asm volatile(\
                                " mrs     %0, cpsr\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define read_vbar()             ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c12, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define write_vbar(val)         asm volatile(\
                                " mcr     p15, 0, %0, c12, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* ARMv7 Registers */
#define HCR_FMO     0x8
#define HCR_IMO     0x10
#define HCR_VI      (0x1 << 7)

/* 32bit case only */
#define read_ttbr0()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr0(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbr1()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbr1(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_ttbcr()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_ttbcr(val)        asm volatile(\
                                " mcr     p15, 0, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair0()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_mair1()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_mair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair0()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair0(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hmair1()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c10, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hmair1(val)       asm volatile(\
                                " mcr     p15, 4, %0, c10, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hsr()              ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c5, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define read_htcr()             ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_htcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hsctlr()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hsctlr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_sctlr()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sctlr(val)       asm volatile(\
                                " mcr     p15, 0, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_httbr()            ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_httbr(val)    asm volatile(\
                            " mcrr     p15, 4, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define read_vtcr()             ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c2, c1, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vtcr(val)         asm volatile(\
                                " mcr     p15, 4, %0, c2, c1, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_vttbr()            ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c2\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_vttbr(val)    asm volatile(\
                            " mcrr     p15, 6, %0, %1, c2\n\t" \
                            : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                            : "memory", "cc")

#define read_hcr()              ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hcr(val)          asm volatile(\
                                " mcr     p15, 4, %0, c1, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_midr()              ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_mpidr()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_vmpidr()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vmpidr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c0, c0, 5\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* Generic Timer */

#define read_cntfrq()           ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntfrq(val)       asm volatile(\
                                " mcr     p15, 0, %0, c14, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthctl()          ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthctl(val)      asm volatile(\
                                " mcr     p15, 4, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_ctl()        ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_ctl(val)    asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cnthp_cval()       ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 6, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_cnthp_cval(val) asm volatile(\
                              " mcrr     p15, 6, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cnthp_tval()       ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cnthp_tval(val)   asm volatile(\
                                " mcr     p15, 4, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntkctl()          ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntkctl(val)      asm volatile(\
                                " mcr     p15, 0, %0, c14, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_ctl()         ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntp_cval()        ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 2, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_cntp_cval(val)  asm volatile(\
                              " mcrr     p15, 2, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cntp_tval()        ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntp_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c2, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntpct()           ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 0, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define read_cntv_ctl()         ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 1\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_ctl(val)     asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 1\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntv_cval()        ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 3, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_cntv_cval(val)  asm volatile(\
                              " mcrr     p15, 3, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_cntv_tval()        ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 0, %0, c14, c3, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_cntv_tval(val)    asm volatile(\
                                " mcr     p15, 0, %0, c14, c3, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cntvct()           ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 1, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define read_cntvoff()          ({ unsigned int v1, v2; asm volatile(\
                                " mrrc     p15, 4, %0, %1, c14\n\t" \
                                : "=r" (v1), "=r" (v2) : : "memory", "cc"); \
                                (((unsigned long long)v2 << 32) + (unsigned long long)v1); })

#define write_cntvoff(val)    asm volatile(\
                              " mcrr     p15, 4, %0, %1, c14\n\t" \
                              : : "r" ((val) & 0xFFFFFFFF), "r" ((val) >> 32) \
                              : "memory", "cc")

#define read_hdfar()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hdfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hifar()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hifar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hpfar()            ({ unsigned int rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 4\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hpfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 4\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* TLB maintenance operations */

/* Invalidate entire unified TLB */
#define invalidate_unified_tlb(val)      asm volatile(\
                " mcr     p15, 0, %0, c8, c7, 0\n\t" \
                : : "r" ((val)) : "memory", "cc")
#endif


