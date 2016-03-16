#include "cpu.h"
#include <arch/armv7.h>
#include <stdint.h>
#include <assert.h>

extern uint32_t __hvc_vector;
void setup_vector()
{
    uint32_t vector_base = (uint32_t) &__hvc_vector;
    write_hvbar(vector_base);
    assert(read_hvbar() == vector_base);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

/* Set Hyp Translation Table Base Register(HTTBR) */
void setup_httbr(addr_t pgtable)
{
    write_httbr((uint64_t) pgtable);
    assert(read_httbr() == pgtable);
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
void setup_mem_attr(void)
{
    uint32_t htcr = 0;

    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    write_htcr(htcr);

    write_hmair0(HMAIR0_VALUE);
    write_hmair1(HMAIR1_VALUE);
}

void enable_mmu(void)
{
    uint32_t hsctlr = 0;

    hsctlr |= (HSCTLR_A | HSCTLR_M);
#ifndef __DISABLE_CACHE__
    hsctlr |= (HSCTLR_I | HSCTLR_C);
#endif
    write_hsctlr(hsctlr);
}
