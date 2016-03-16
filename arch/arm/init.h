#ifndef __CPU_INIT_H__
#define __CPU_INIT_H__

#include <stdint.h>
#include <asm/asm.h>
#include <arch/armv7.h>

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")

void start_hypervisor(void);

#endif // __CPU_H__
