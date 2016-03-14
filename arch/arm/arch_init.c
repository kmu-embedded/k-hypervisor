#include "arch_init.h"
#include "mm.h"

extern uint32_t __hvc_vector;
void SECTION(".init.arch") cpu_init()
{
    uint32_t vector_base = (uint32_t) &__hvc_vector;
    write_hvbar(vector_base);
    assert(read_hvbar() == vector_base);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

#include "gic-v2.h"
#include "vgic.h"
void SECTION(".init.arch") irq_init()
{
    // TODO(casionwoo): add a init function of irq handler table for hypervisor.
    gic_init();

    vgic_init();
    virq_table_init();
    vgic_enable(1);

    write_hcr(0x10 | 0x8);
}
