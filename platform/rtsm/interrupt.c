#include <irq-chip.h>
#include <drivers/gic-v2.h>
#include <arch/armv7/smp.h>

struct irq_chip gic_v2 = {
    .init = gic_init,
    .enable = gic_enable_irq,
    .disable = gic_disable_irq,
    .ack = gic_get_irq_number,
    .eoi = gic_completion_irq,
    .dir = gic_deactivate_irq,
    .set_irq_type = gic_configure_irq,
};

struct virq_chip vgic_v2 = {
    .init = gich_init,
    .enable = gich_enable,
    .disable = gich_disable,
    .forward_pending_irq = gic_inject_pending_irqs,
    .forward_irq = virq_inject,
};

void set_irqchip_type(void)
{
    uint8_t cpuid = smp_processor_id();
    irq_hw = &gic_v2;
    virq_hw = &vgic_v2;

    irq_hw->init();
    if (cpuid == 0) {
        virq_hw->init();
    }
}
