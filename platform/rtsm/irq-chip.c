#include <irq-chip.h>
#include "../../arch/arm/gic-v2.h"

struct irq_chip_t gicv2 = {
    .init = gic_init,
    .enable = gic_enable_irq,
    .disable = gic_disable_irq,
    .ack = gic_get_irq_number,
    .eoi = gic_completion_irq,
    .dir = gic_deactivate_irq,
    .set_irq_type = gic_configure_irq,
    .forward = virq_inject,
};

void register_irq_chip()
{
    irq_chip = &gicv2;
}
