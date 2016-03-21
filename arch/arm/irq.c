#include <stdio.h>
#include <arch/irq.h>
#include <config.h>
#include <arch/armv7.h>

#include <irq-chip.h>
#include "../../include/types.h"

#define VIRQ_MIN_VALID_PIRQ     16
#define VIRQ_NUM_MAX_PIRQS      MAX_IRQS

static irq_handler_t irq_handlers[MAX_IRQS];

#include <core/vm/virq.h>
#include <arch_regs.h>

hvmm_status_t do_irq(struct core_regs *regs)
{
    uint32_t irq = irq_hw->ack();

#if 0
    if (irq == 34) {
        printf("SP804_TIMER IRQ!!\n");
    }
#endif

    irq_hw->eoi(irq);

    is_guest_irq(irq);

    if (irq_handlers[irq]) {
        irq_handlers[irq](irq, (struct arch_regs *)regs, 0);
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

