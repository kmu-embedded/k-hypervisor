#include "cpu.h"
#include <arch/armv7.h>
#include <stdint.h>
#include <assert.h>

extern uint32_t __hvc_vector;
void setup_vector()
{
    write_cp32((uint32_t) &__hvc_vector, HVBAR);
    assert(read_cp32(HVBAR) == (uint32_t) &__hvc_vector);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

/* Set Hyp Translation Table Base Register(HTTBR) */
void setup_httbr(addr_t pgtable)
{
    write_cp64((uint64_t) pgtable, HTTBR);
    assert(read_cp64(HTTBR) == pgtable);
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
void setup_mem_attr(void)
{
    uint32_t htcr = 0;

    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    write_cp32(htcr, HTCR);

    write_cp32(HMAIR0_VALUE, HMAIR0);
    write_cp32(HMAIR1_VALUE, HMAIR1);
}

void enable_mmu(void)
{
    uint32_t hsctlr = 0;

    hsctlr |= (HSCTLR_A | HSCTLR_M);
#ifndef __DISABLE_CACHE__
    hsctlr |= (HSCTLR_I | HSCTLR_C);
#endif
    write_cp32(hsctlr, HSCTLR);
}

/* Set Hyp System Trap Register(HSTR) */
void enable_traps(void)
{
    uint32_t hstr = 0;

    hstr = 1 << 0;
    write_cp32(hstr, HSTR);
}
