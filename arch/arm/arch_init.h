#ifndef __ARCH_INIT_H__
#define __ARCH_INIT_H__

#include <stdint.h>
#include <arch/armv7.h>    //smp_processor_id()
#include <assert.h>
#include <asm/asm.h>

void cpu_init();
void irq_init();

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")

#endif
