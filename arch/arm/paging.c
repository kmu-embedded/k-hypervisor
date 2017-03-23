#include "paging.h"
#include <io.h>
#include <arch/armv7.h>

void paging_create(addr_t base)
{
    int i = 0;

    uint32_t second_base = base + 0x1000;
    uint32_t third_base = second_base + 0x4000;

    for (i = 0; i < L1_ENTRY; i++) {
        write64(set_table(second_base + GET_L2_INDEX(i)).raw, base + GET_OFFSET(i));
    }

    for (i = 0; i < L1_ENTRY * L2_ENTRY; i++) {
        write64(set_table(third_base + GET_L2_INDEX(i)).raw, second_base + GET_OFFSET(i));
    }
}

void paging_add_mapping(addr_t va, addr_t pa, uint8_t mem_attr, uint32_t size)
{
    int i = 0;
    uint64_t httbr = read_cp64(HTTBR);

    for (i = 0; i < size; i += 0x1000, va += 0x1000, pa += 0x1000) {
        write_pgentry((addr_t) httbr, va >> PAGE_SHIFT, pa, mem_attr, 0, 1);
    }
}

void paging_add_ipa_mapping(addr_t base, addr_t ipa, addr_t pa, uint8_t mem_attr, uint8_t af, uint32_t size)
{
    int i = 0;

    for (i = 0; i < size; i += 0x1000, ipa += 0x1000, pa += 0x1000) {
        write_pgentry(base, ipa >> PAGE_SHIFT, pa, mem_attr, 3, af);
    }
}
