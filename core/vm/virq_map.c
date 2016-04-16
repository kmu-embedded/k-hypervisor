#include <debug.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <core/vm/vcpu.h>

uint32_t virq_to_pirq(struct vcpu *v, uint32_t virq)
{
    return v->map[virq].pirq;
}

uint32_t pirq_to_virq(struct vcpu *v, uint32_t pirq)
{
    return v->map[pirq].virq;
}

void virq_enable(struct vcpu *v, uint32_t pirq, uint32_t virq)
{
    v->map[virq].pirq    = pirq;
    v->map[virq].enabled = GUEST_IRQ_ENABLE;
}

void virq_disable(struct vcpu *v, uint32_t virq)
{
    v->map[virq].enabled = GUEST_IRQ_DISABLE;
}

void pirq_enable(struct vcpu *v, uint32_t pirq, uint32_t virq)
{
    v->map[pirq].virq    = virq;
    v->map[pirq].enabled = GUEST_IRQ_ENABLE;
}

void pirq_disable(struct vcpu *v, uint32_t pirq)
{
    v->map[pirq].enabled = GUEST_IRQ_DISABLE;
}



#include <arch/armv7.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>
#include <irq-chip.h>

bool is_guest_irq(uint32_t irq)
{
    bool result = false;
    uint32_t virq;
    struct vcpu *vcpu;
    uint32_t pcpu = smp_processor_id();
    struct running_vcpus_entry_t *sched_vcpu_entry;

    list_for_each_entry(struct running_vcpus_entry_t, sched_vcpu_entry, &__running_vcpus[pcpu], head) {
        vcpu = vcpu_find(sched_vcpu_entry->vcpuid);
        virq = pirq_to_virq(vcpu, irq);

        virq = (virq == VIRQ_INVALID ? irq : virq);

        if (virq == VIRQ_INVALID) {
            continue;
        }

        result = virq_hw->forward_irq(vcpu->vcpuid, virq, irq, INJECT_SW);
    }

    return result;
}

