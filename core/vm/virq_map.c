#include <debug.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <core/vm/vcpu.h>

uint32_t virq_to_pirq(struct vcpu *v, uint32_t virq)
{
    return v->map[virq].pirq;
}

uint32_t pirq_to_enabled_virq(struct vcpu *v, uint32_t pirq)
{
    uint32_t virq = VIRQ_INVALID;

    if (v->map[pirq].enabled)
        virq = v->map[pirq].virq;

    return virq;
}

uint32_t virq_to_enabled_pirq(struct vcpu *v, uint32_t virq)
{
    uint32_t pirq = PIRQ_INVALID;

    if (v->map[virq].enabled)
        pirq = v->map[virq].pirq;

    return pirq;
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

    //if(irq == 34) printf(" SP804 INJECT?\n");

    list_for_each_entry(struct running_vcpus_entry_t, sched_vcpu_entry, &__running_vcpus[pcpu], head) {
        vcpu = vcpu_find(sched_vcpu_entry->vcpuid);
        virq = pirq_to_enabled_virq(vcpu, irq);
        if (virq == VIRQ_INVALID) {
            continue;
        }
        result = virq_hw->forward_irq(vcpu->vcpuid, virq, irq, INJECT_HW);
    }

    return result;
}

