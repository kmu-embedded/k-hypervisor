#include <core/vm/virq.h>
#include <debug.h>
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include "../../drivers/gic-v2.h"



#include <core/scheduler.h>

#define SET_VIRQMAP(map, _pirq, _virq) \
    do {                                 \
        map[_pirq].virq = _virq;   \
        map[_virq].pirq = _pirq;   \
    } while (0)

void virq_create(struct virq *virq)
{
    int i;

    for (i = 0; i < MAX_IRQS; i++) {
        virq->map[i].enabled = GUEST_IRQ_DISABLE;
        virq->map[i].virq = VIRQ_INVALID;
        virq->map[i].pirq = PIRQ_INVALID;
    }

    for (i = 0; i < MAX_PENDING_VIRQS; i++) {
        virq->pending_irqs[i].valid = 0;
    }
    gicd_banked_regs_create(&virq->gicd_banked_regs);
}

void vgic_init_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        status->lr[i] = 0;
    }

    status->hcr = 0;
    status->apr = 0;
    status->vmcr = 0;
}

void virq_init(struct virq *virq, vmid_t vmid)
{
    // virq virq->mapping for serial
    switch (vmid) {
        case 0:
            SET_VIRQMAP(virq->map, 38, 37);
            break;

        case 1:
            SET_VIRQMAP(virq->map, 39, 37);
            break;

        case 2:
            SET_VIRQMAP(virq->map, 40, 37);
            break;

        default:
            debug_print("virq_create error!\n");
            break;
    }

    /* vgic_status initialization */
    memset(&virq->vgic_status, 0, sizeof(struct vgic_status));
    gicd_banked_regs_init(&virq->gicd_banked_regs);
}

uint32_t pirq_to_virq(struct virq *v, uint32_t pirq)
{
    return v->map[pirq].virq;
}

uint32_t virq_to_pirq(struct virq *v, uint32_t virq)
{
    return v->map[virq].pirq;
}

uint32_t pirq_to_enabled_virq(struct virq *v, uint32_t pirq)
{
    uint32_t virq = VIRQ_INVALID;

    if (v->map[pirq].enabled)
        virq = v->map[pirq].virq;

    return virq;
}

uint32_t virq_to_enabled_pirq(struct virq *v, uint32_t virq)
{
    uint32_t pirq = PIRQ_INVALID;

    if (v->map[virq].enabled)
        pirq = v->map[virq].pirq;

    return pirq;
}

void virq_enable(struct virq *v, uint32_t pirq, uint32_t virq)
{
    v->map[virq].pirq    = pirq;
    v->map[virq].enabled = GUEST_IRQ_ENABLE;
}

void virq_disable(struct virq *v, uint32_t virq)
{
    v->map[virq].enabled = GUEST_IRQ_DISABLE;
}

void pirq_enable(struct virq *v, uint32_t pirq, uint32_t virq)
{
    v->map[pirq].virq    = virq;
    v->map[pirq].enabled = GUEST_IRQ_ENABLE;
}

void pirq_disable(struct virq *v, uint32_t pirq)
{
    v->map[pirq].enabled = GUEST_IRQ_DISABLE;
}

#include <arch/gic_regs.h>
#include <irq-chip.h>

void virq_save(struct virq *virq)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        virq->vgic_status.lr[i] = GICH_READ(GICH_LR(i));
    }

    virq->vgic_status.hcr = GICH_READ(GICH_HCR);
    virq->vgic_status.apr = GICH_READ(GICH_APR);
    virq->vgic_status.vmcr = GICH_READ(GICH_VMCR);

    virq_hw->disable();
}

void virq_restore(struct virq *virq, vmid_t vmid)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        GICH_WRITE(GICH_LR(i), virq->vgic_status.lr[i]);
    }

    GICH_WRITE(GICH_APR, virq->vgic_status.apr);
    GICH_WRITE(GICH_VMCR, virq->vgic_status.vmcr);
    GICH_WRITE(GICH_HCR, virq->vgic_status.hcr);

    virq_hw->forward_pending_irq(vmid);
    virq_hw->enable();
}

#include <arch/armv7.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>

bool is_guest_irq(uint32_t irq)
{
    bool result = false;
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
        result = virq_hw->forward_irq(vcpu->vcpuid, virq, irq, INJECT_HW);
    }

    return result;
}

