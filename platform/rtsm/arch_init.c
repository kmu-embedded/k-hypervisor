#include "arch_init.h"
#include "../../arch/arm/mm.h"
#include <irq-chip.h>

extern uint32_t __hvc_vector;
void SECTION(".init.arch") arch_cpu_init()
{
    uint32_t vector_base = (uint32_t) &__hvc_vector;
    write_hvbar(vector_base);
    assert(read_hvbar() == vector_base);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

#include "../../arch/arm/gic-v2.h"
void SECTION(".init.arch") irq_chip_init()
{
    // TODO(casionwoo): add a init function of irq handler table for hypervisor.
    register_irq_chip();
    irq_chip->init();

    write_hcr(0x10 | 0x8);
}
