#include <core/interrupt.h>
#include <hvmm_types.h>
#include <arch/armv7.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>
#include <rtsm-config.h>

#include "../platform/rtsm/arch_init.h"
#include <irq-chip.h>
#define VIRQ_MIN_VALID_PIRQ     16
#define VIRQ_NUM_MAX_PIRQS      MAX_IRQS

static interrupt_handler_t interrupt_handlers[MAX_IRQS];

static void interrupt_inject_enabled_guest(uint32_t irq)
{
    uint32_t virq;
    struct vcpu *vcpu;
    uint32_t pcpu = smp_processor_id();
    struct running_vcpus_entry_t *sched_vcpu_entry;

    list_for_each_entry(struct running_vcpus_entry_t, sched_vcpu_entry, &__running_vcpus[pcpu], head) {
        vcpu = vcpu_find(sched_vcpu_entry->vcpuid);
        virq = pirq_to_enabled_virq(&vcpu->virq, irq);
        if (virq == VIRQ_INVALID) {
            continue;
        }

        //virq_inject(vcpu->vcpuid, virq, irq, INJECT_HW);
        virq_hw->forward_irq(vcpu->vcpuid, virq, irq, INJECT_HW);
    }
}

void interrupt_init()
{
    irq_chip_init();
}

void register_irq_handler(uint32_t irq, interrupt_handler_t handler)
{
    if (irq < MAX_IRQS)
        interrupt_handlers[irq] = handler;
}

void interrupt_service_routine(int irq, void *current_regs)
{

    /* TODO(casionwoo) :
    *   PPI, SGI : Just inject or call hyp's handler
    *
    *   SPI : (pcpu == 0) : send to other cpus and call hyp's handler
    *         (pcpu != 0) : inject to guests or call hyp's handler
    */

    struct arch_regs *regs = (struct arch_regs *)current_regs;

    irq_hw->eoi(irq);
    interrupt_inject_enabled_guest(irq);

    /* Host irq */
    if (interrupt_handlers[irq]) {
        interrupt_handlers[irq](irq, regs, 0);
        irq_hw->dir(irq);
    }
}

