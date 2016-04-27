#ifndef __CPU_H__
#define __CPU_H__

#include "lpae.h"

void setup_vector();
void setup_mem_attr(void);
void setup_httbr(addr_t pgtable);
void enable_mmu(void);
void enable_traps(void);

#endif // __CPU_H__
