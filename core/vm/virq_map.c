#include <debug.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <core/vm/vcpu.h>
#include <core/vm/vm.h>

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
#include <core/sched/scheduler_skeleton.h>
#include <irq-chip.h>
#include <arch/irq.h>

static irqreturn_t is_guest_sgi(int irq, void *pregs, void *pdata)
{
    printf("SGI[%d] is not handled in khypervisor\n", irq);
    return VM_IRQ;
}

static irqreturn_t is_guest_ppi(int irq, void *pregs, void *pdata)
{
    struct vcpu *vcpu;
    uint32_t virq;
    uint32_t pcpu = smp_processor_id();
    struct sched_entry *rve;
    struct list_head *rvs_list = &sched[pcpu]->inflight_entries;

    list_for_each_entry(struct sched_entry, rve, rvs_list, head_inflight) {
        vcpu = vcpu_find(rve->vcpuid);
        virq = pirq_to_virq(vcpu, irq);

        if (virq == VIRQ_INVALID) {
            continue;
        }

        virq_hw->forward_irq(vcpu, virq, irq, INJECT_SW);
    }

    return VM_IRQ;
}

static irqreturn_t is_guest_spi(int irq, void *pregs, void *pdata)
{
    struct vcpu *vcpu = get_current_vcpu();
    uint32_t virq = pirq_to_virq(vcpu, irq);

    if (virq != VIRQ_INVALID) {
        virq_hw->forward_irq(vcpu, virq, irq, INJECT_SW);
    }

    return VM_IRQ;
}

void irq_handler_init(irq_handler_t *handler)
{
    int i;

    for (i = 0; i < 16; i++){
        if (handler[i])
            continue;
        handler[i] = is_guest_sgi;
    }

    for (i = 16; i < 32; i++){
        if (handler[i])
            continue;
        handler[i] = is_guest_ppi;
    }

    for (i = 32; i < 1025; i++){
        if (handler[i])
            continue;
        handler[i] = is_guest_spi;
    }
}

