#include "cpu.h"
#include <arch/armv7.h>
#include <stdint.h>
#include <assert.h>

extern uint32_t __hvc_vector;
void setup_vector()
{
    uint32_t hvbar = 0;
    WRITE_CP32((uint32_t) &__hvc_vector, HVBAR);
    READ_CP32(hvbar, HVBAR);
    assert(hvbar == (uint32_t) &__hvc_vector);

    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

/* Set Hyp Translation Table Base Register(HTTBR) */
void setup_httbr(addr_t pgtable)
{
    uint64_t httbr = 0;
    WRITE_CP64((uint64_t) pgtable, HTTBR);
    READ_CP64(httbr, HTTBR);
    assert(httbr == pgtable);
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
void setup_mem_attr(void)
{
    uint32_t htcr = 0;

    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    WRITE_CP32(htcr, HTCR);
    //write_htcr(htcr);

    WRITE_CP32(HMAIR0_VALUE, HMAIR0);
    WRITE_CP32(HMAIR1_VALUE, HMAIR1);
    //write_hmair0(HMAIR0_VALUE);
    //write_hmair1(HMAIR1_VALUE);
}

void enable_mmu(void)
{
    uint32_t hsctlr = 0;

    hsctlr |= (HSCTLR_A | HSCTLR_M);
#ifndef __DISABLE_CACHE__
    hsctlr |= (HSCTLR_I | HSCTLR_C);
#endif
    WRITE_CP32(hsctlr, HSCTLR);
    //write_hsctlr(hsctlr);
}

