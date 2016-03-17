#ifndef __CPU_H__
#define __CPU_H__

#include "lpae.h"

void setup_vector();
void setup_mem_attr(void);
void setup_httbr(addr_t pgtable);
void enable_mmu(void);

#include "../../include/asm/macro.h"

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")


#endif // __CPU_H__
