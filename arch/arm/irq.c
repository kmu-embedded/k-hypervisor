#include <stdio.h>
#include <arch/irq.h>
#include <config.h>
#include <core/vm/vcpu.h>
#include <arch/armv7/smp.h>
#include <irq-chip.h>
#include <types.h>
#include <core/scheduler.h>

#define VIRQ_MIN_VALID_PIRQ     16
#define VIRQ_NUM_MAX_PIRQS      MAX_IRQS

static irq_handler_t irq_handlers[MAX_IRQS];
static irq_handler_t vdev_irq_handlers[MAX_IRQS];

hvmm_status_t do_irq(struct core_regs *regs)
{
    uint32_t irq = irq_hw->ack();
    uint32_t pcpu = smp_processor_id();

//    printf("pcpu[%d]: irq %d\n", pcpu, irq);
//    if(irq > 16) irq_hw->eoi(irq);

    irq_hw->eoi(irq);

    if (irq != 26)
        printf("irq[%d]\n", irq);

    if (irq < 16) {
        // SGI Handler
        printf("CPU[%d] SGI[%d] Occurred\n",pcpu ,irq);
    } else if (irq_handlers[irq]) {
        // Handler for Hypervisor
        irq_handlers[irq](irq, regs, 0);
        irq_hw->dir(irq);
    } else if (vdev_irq_handlers[irq]) {
        // Handler for VMs
        vdev_irq_handlers[irq](irq, regs, 0);
    } else {
        // Not found handler that just forward irq to VMs
        is_guest_irq(irq);
    }

    return HVMM_STATUS_SUCCESS;
}

void irq_init()
{
    set_irqchip_type();
    write_cp32((0x10 | 0x8), HCR);
}

void register_irq_handler(uint32_t irq, irq_handler_t handler, uint8_t polarity)
{
    if (irq < MAX_IRQS) {
        irq_handlers[irq] = handler;
    }
    irq_hw->set_irq_type(irq, polarity);
    irq_hw->enable(irq);
}

void register_vdev_irq_handler(uint32_t irq, irq_handler_t handler)
{
    if (irq < MAX_IRQS) {
        vdev_irq_handlers[irq] = handler;
    }
}
