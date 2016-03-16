#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <asm/asm.h>
#include <assert.h>
#include <arch/armv7.h>

void cpu_init(void);

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")

extern uint32_t __hvc_vector;

#endif // __CPU_H__
