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
#include <irq-chip.h>

void is_guest_irq(uint32_t irq)
{
    struct vcpu *vcpu;
    struct vmcb *vm;
    uint32_t virq;
    uint32_t pcpu = smp_processor_id();
    struct list_head *vm_list = get_all_vms();

    if (irq < 32) { // PPI
        struct running_vcpus_entry_t *rve;
        struct list_head *rvs_list = &__running_vcpus[pcpu];

        list_for_each_entry(struct running_vcpus_entry_t, rve, rvs_list, head) {
            vcpu = vcpu_find(rve->vcpuid);
            virq = pirq_to_virq(vcpu, irq);

            if (virq == VIRQ_INVALID) {
                continue;
            }

            virq_hw->forward_irq(vcpu, virq, irq, INJECT_SW);
        }
    } else if (irq < 1024) { //SPI

        list_for_each_entry(struct vmcb, vm, vm_list, head) {
            vcpu = vm->vcpu[0];
            virq = pirq_to_virq(vcpu, irq);

            if (virq == VIRQ_INVALID || vm->state != RUNNING) {
                continue;
            }

            virq_hw->forward_irq(vcpu, virq, irq, INJECT_SW);
        }
    }
}

