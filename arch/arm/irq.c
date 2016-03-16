#include <core/irq.h>
#include <hvmm_types.h>
#include <arch/armv7.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>
#include <rtsm-config.h>

#include "../platform/rtsm/arch_init.h"
#include <irq-chip.h>
#define VIRQ_MIN_VALID_PIRQ     16
#define VIRQ_NUM_MAX_PIRQS      MAX_IRQS

static irq_handler_t irq_handlers[MAX_IRQS];

static void irq_inject_enabled_guest(uint32_t irq)
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

void irq_init()
{
    irq_chip_init();
    write_hcr(0x10 | 0x8);
}

void register_irq_handler(uint32_t irq, irq_handler_t handler)
{
    if (irq < MAX_IRQS)
        irq_handlers[irq] = handler;
}

void irq_service_routine(int irq, void *current_regs)
{

    /* TODO(casionwoo) :
    *   PPI, SGI : Just inject or call hyp's handler
    *
    *   SPI : (pcpu == 0) : send to other cpus and call hyp's handler
    *         (pcpu != 0) : inject to guests or call hyp's handler
    */

    struct arch_regs *regs = (struct arch_regs *)current_regs;

    irq_hw->eoi(irq);
    irq_inject_enabled_guest(irq);

    /* Host irq */
    if (irq_handlers[irq]) {
        irq_handlers[irq](irq, regs, 0);
        irq_hw->dir(irq);
    }
}

