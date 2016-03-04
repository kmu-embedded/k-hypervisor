#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>
#include <stdbool.h>

struct memmap_desc {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    uint8_t attr;
};

void set_pgtable(struct memmap_desc *desc);
void pgtable_init(void);
void enable_mmu();
void write_pgentry(void *pgtable_base, struct memmap_desc *mem_desc, bool is_guest);
void write_pgentry_4k(void *pgtable_base, struct memmap_desc *mem_desc, bool is_guest);
void write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size);

#endif /* __MM_H__*/
