#include <stdio.h>
#include <arch/irq.h>
#include <config.h>
#include <core/vm/vcpu.h> // is_guest_irq

#include <irq-chip.h>
#include <types.h>

#define VIRQ_MIN_VALID_PIRQ     16
#define VIRQ_NUM_MAX_PIRQS      MAX_IRQS

static irq_handler_t irq_handlers[MAX_IRQS];

#include <arch_regs.h>

hvmm_status_t do_irq(struct core_regs *regs)
{
    uint32_t irq = irq_hw->ack();

    irq_hw->eoi(irq);

    is_guest_irq(irq);

    if (irq_handlers[irq]) {
        irq_handlers[irq](irq, regs, 0);
        irq_hw->dir(irq);
    }
    return HVMM_STATUS_SUCCESS;
}

void irq_init()
{
    set_irqchip_type();
    write_hcr(0x10 | 0x8); // enable irq
}

void register_irq_handler(uint32_t irq, irq_handler_t handler)
{
    if (irq < MAX_IRQS)
        irq_handlers[irq] = handler;
}

