#ifndef __STAGE2_MM_H__
#define __STAGE2_MM_H__

void stage2_mm_create(uint32ptr_t *pgtable_base);
void stage2_mm_init(struct memdesc_t **mdlist, char **_vmid_ttbl, vmid_t vmid);
void guest_memory_init_mmu(void);

#endif //__STAGE2_MM_H__
