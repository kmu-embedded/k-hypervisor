#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>
#include <stdbool.h>

struct memdesc_t {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    uint8_t attr;
};

void set_hmair(void);
void set_htcr(void);
void set_httbr(void);
void enable_mmu(void);

void dump_pgtable(void);
void pgtable_init(uint32_t base);
void set_pgtable(struct memdesc_t *desc);
void write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size);

void vm_pgtable_init(uint32_t base);
void write_vm_pgentry(void *pgtable_base, uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size, bool is_guest);


#endif /* __MM_H__*/
