#ifndef __ARMV7_SYSCTRL_H__
#define __ARMV7_SYSCTRL_H__

#include <stdint.h>
#include <asm/asm.h>


#define read_vbar()             ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c12, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })
#define write_vbar(val)         asm volatile(\
                                " mcr     p15, 0, %0, c12, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hsr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c5, c2, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_sctlr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_sctlr(val)       asm volatile(\
                                " mcr     p15, 0, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define HCR_FMO     0x8
#define HCR_IMO     0x10
#define HCR_VI      (0x1 << 7)

#define read_hcr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c1, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hcr(val)          asm volatile(\
                                " mcr     p15, 4, %0, c1, c1, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")


#define read_midr()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_mpidr()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 0, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define read_vmpidr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c0, c0, 5\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_vmpidr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c0, c0, 5\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define HSCTLR_TE        (1 << 30)      /**< Thumb Exception enable. */
#define HSCTLR_EE        (1 << 25)      /**< Exception Endianness. */
#define HSCTLR_FI        (1 << 21)      /**< Fast Interrupts configuration enable. */
#define HSCTLR_WXN       (1 << 19)      /**< Write permission emplies XN. */
#define HSCTLR_I         (1 << 12)      /**< Instruction cache enable.  */
#define HSCTLR_CP15BEN   (1 << 7)       /**< In ARMv7 this bit is RAZ/SBZP. */
#define HSCTLR_C         (1 << 2)       /**< Data or unified cache enable. */
#define HSCTLR_A         (1 << 1)       /**< Alignment check enable. */
#define HSCTLR_M         (1 << 0)       /**< MMU enable. */
#define HSCTLR_BASE      0x30c51878     /**< HSTCLR Base address */


#define read_hsctlr()           ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c1, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hsctlr(val)       asm volatile(\
                                " mcr     p15, 4, %0, c1, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_cbar()              ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c15, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#endif /* armv7-sysctrl.h */
