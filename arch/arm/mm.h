#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>
#include <arch/armv7.h>
#include <asm/asm.h>

#include <rtsm-config.h>

struct memdesc_t {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    uint8_t attr;
};

void mm_init();
void add_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size);
void enable_mmu(void);

void pgtable_init(uint32_t pgtable_base);
#endif /* __MM_H__*/
