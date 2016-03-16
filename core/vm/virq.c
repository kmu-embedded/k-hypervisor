#include <core/vm/virq.h>
#include <debug_print.h>
#include <rtsm-config.h>
#include <stdlib.h>

#define SET_VIRQMAP(map, _pirq, _virq) \
    do {                                 \
        map[_pirq].virq = _virq;   \
        map[_virq].pirq = _pirq;   \
    } while (0)

void virq_create(struct virq *virq)
{
    int i;
    struct virqmap_entry *map;
    struct virq_entry *entry;

    virq->guest_virqmap = malloc(sizeof(struct guest_virqmap));

    map = virq->guest_virqmap->map;
    for (i = 0; i < MAX_IRQS; i++) {
        map[i].enabled = GUEST_IRQ_DISABLE;
        map[i].virq = VIRQ_INVALID;
        map[i].pirq = PIRQ_INVALID;
    }

    entry = virq->pending_irqs;
    for (i = 0; i < MAX_PENDING_VIRQS; i++) {
        entry[i].valid = 0;
    }
}

hvmm_status_t vgic_init_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        status->lr[i] = 0;
    }

    status->hcr = 0;
    status->apr = 0;
    status->vmcr = 0;

    return HVMM_STATUS_SUCCESS;
}

void virq_init(struct virq *virq, vmid_t vmid)
{
    struct virqmap_entry *map;

    map = virq->guest_virqmap->map;

    SET_VIRQMAP(map, 1, 1);
    SET_VIRQMAP(map, 16, 16);
    SET_VIRQMAP(map, 17, 17);
    SET_VIRQMAP(map, 18, 18);
    SET_VIRQMAP(map, 19, 19);
    SET_VIRQMAP(map, 31, 31);
    SET_VIRQMAP(map, 32, 32);
    SET_VIRQMAP(map, 33, 33);
    SET_VIRQMAP(map, 34, 34);
    SET_VIRQMAP(map, 35, 35);
    SET_VIRQMAP(map, 36, 36);
    SET_VIRQMAP(map, 41, 41);
    SET_VIRQMAP(map, 42, 42);
    SET_VIRQMAP(map, 43, 43);
    SET_VIRQMAP(map, 44, 44);
    SET_VIRQMAP(map, 45, 45);
    SET_VIRQMAP(map, 46, 46);
    SET_VIRQMAP(map, 47, 47);
    SET_VIRQMAP(map, 69, 69);

    // virq mapping for serial
    switch (vmid) {
        case 0:
            SET_VIRQMAP(map, 38, 37);
            break;

        case 1:
            SET_VIRQMAP(map, 39, 37);
            break;

        case 2:
            SET_VIRQMAP(map, 40, 37);
            break;

        default:
            debug_print("virq_create error!\n");
            break;
    }

    /* vgic_status initialization */
    vgic_init_status(&virq->vgic_status);
}

uint32_t pirq_to_virq(struct virq *v, uint32_t pirq)
{
    struct virqmap_entry *map = v->guest_virqmap->map;

    return map[pirq].virq;
}

uint32_t virq_to_pirq(struct virq *v, uint32_t virq)
{
    struct virqmap_entry *map = v->guest_virqmap->map;

    return map[virq].pirq;
}

uint32_t pirq_to_enabled_virq(struct virq *v, uint32_t pirq)
{
    uint32_t virq = VIRQ_INVALID;
    struct virqmap_entry *map = v->guest_virqmap->map;

    if (map[pirq].enabled)
        virq = map[pirq].virq;

    return virq;
}

uint32_t virq_to_enabled_pirq(struct virq *v, uint32_t virq)
{
    uint32_t pirq = PIRQ_INVALID;
    struct virqmap_entry *map = v->guest_virqmap->map;

    if (map[virq].enabled)
        pirq = map[virq].pirq;

    return pirq;
}

void virq_enable(struct virq *v, uint32_t virq)
{
    struct virqmap_entry *map = v->guest_virqmap->map;

    map[virq].enabled = GUEST_IRQ_ENABLE;
}

#include "vgic.h"
#include <arch/gic_regs.h>
#include <irq-chip.h>

hvmm_status_t virq_save(struct virq *virq)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        virq->vgic_status.lr[i] = GICH_READ(GICH_LR(i));
    }

    virq->vgic_status.hcr = GICH_READ(GICH_HCR);
    virq->vgic_status.apr = GICH_READ(GICH_APR);
    virq->vgic_status.vmcr = GICH_READ(GICH_VMCR);

    virq_hw->disable();

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid)
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

    return HVMM_STATUS_SUCCESS;
}

