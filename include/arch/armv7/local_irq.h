#ifndef __LOCAL_IRQ__
#define __LOCAL_IRQ__

#include <stdint.h>
#include <asm/asm.h>

#define asm_clz(x)      ({ uint32_t rval; asm volatile(\
                         " clz %0, %1\n\t" \
                         : "=r" (rval) : "r" (x) : ); rval; })

#define irq_enable() 	asm volatile("cpsie i" : : : "memory")
#define irq_disable() 	asm volatile ("cpsid i" : : : "memory")

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
