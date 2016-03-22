#ifndef __ARM_INLINE__
#define __ARM_INLINE__

#include <stdint.h>
#include <asm/asm.h>

#define sev()   asm __volatile__ ("sev" : : : "memory")
#define wfe()   asm __volatile__ ("wfe" : : : "memory")
#define wfi()   asm __volatile__ ("wfi" : : : "memory")

#define isb() asm __volatile__ ("isb" : : : "memory")
#define dsb() asm __volatile__ ("dsb" : : : "memory")
#define dmb() asm __volatile__ ("dmb" : : : "memory")

#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                         " clz %0, %1\n\t" \
                         : "=r" (rval) : "r" (x) : ); rval; })

#define irq_enable() asm volatile("cpsie i" : : : "memory")
#define irq_disable() asm volatile ("cpsid i" : : : "memory")

#define irq_disabled() ({ unsigned long tf; \
                          asm volatile (" mrs     %0, cpsr\n\t" \
                          : "=r" (tf) \
                          :  \
                          : "memory", "cc"); \
                          (tf & CPSR_IRQ_DISABLED) ? TRUE : FALSE; })


#define irq_save(flags)    do { \
                    asm volatile ( \
                    "mrs     %0, cpsr\n\t" \
                    "cpsid   i\n\t" \
                    : "=r" ((flags)) : : "memory", "cc"); \
                    } while (0)


#define irq_restore(flags) do { \
                    asm volatile (" msr     cpsr_c, %0" \
                    : : "r" ((flags)) : "memory", "cc"); \
                    } while (0)


#endif
