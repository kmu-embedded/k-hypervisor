#include <core/vm/virq.h>
#include <debug_print.h>
#include <rtsm-config.h>
#include <stdlib.h>

#define SET_VIRQMAP(map, _pirq, _virq) \
    do {                                 \
        map[_pirq].virq = _virq;   \
        map[_virq].pirq = _pirq;   \
    } while (0)

/* TODO(igxactly): move code from guest_interrupt_init --> virq_setup, virq_create */
void virq_create(struct virq *virq)
{
    /* TODO(igkang): something like virq_setup will be called here
     *  to load configurations from DTB or config data */

    /* Initialize PIRQ to VIRQ mapping */
    int i;
    struct virqmap_entry *map;

    virq->guest_virqmap = malloc(sizeof(struct guest_virqmap));

    map = virq->guest_virqmap->map;
    for (i = 0; i < MAX_IRQS; i++) {
        map[i].enabled = GUEST_IRQ_DISABLE;
        map[i].virq = VIRQ_INVALID;
        map[i].pirq = PIRQ_INVALID;
    }
}

hvmm_status_t virq_init(struct virq *virq, vmid_t vmid)
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
    return vgic_init_status(&virq->vgic_status);
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

hvmm_status_t virq_save(struct virq *virq)
{
    return vgic_save_status(&virq->vgic_status);
}

hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid)
{
    return vgic_restore_status(&virq->vgic_status, vmid);
}

