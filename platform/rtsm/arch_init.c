#include <irq-chip.h>
#include "arch_init.h"

void irq_chip_init()
{
    // TODO(casionwoo): add a init function of irq handler table for hypervisor.
    register_irq_chip();
    irq_hw->init();
    virq_hw->init();
}
