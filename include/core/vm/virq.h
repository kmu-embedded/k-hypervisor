#ifndef __VIRQ_H__
#define __VIRQ_H__

#include <hvmm_types.h>
//#include <stdint.h>
// TODO(wonseok): make it neat.
#include "../../../arch/arm/vgic.h"
#include <core/interrupt.h>

/**
 * @breif   Saves a mapping information to find a virq for injection.
 *
 * We do not consider a sharing device that's why we save only one vmid.
 * @todo    (wonseok): need to change a structure when we support
 *                     a sharing device among guests.
 */
struct virqmap_entry {
    uint32_t enabled;   /**< virqmap enabled flag */
    uint32_t virq;      /**< Virtual interrupt nubmer */
    uint32_t pirq;      /**< Pysical interrupt nubmer */
};

struct guest_virqmap {
    struct virqmap_entry map[1024];
};

struct virq {
    struct vgic_status vgic_status;
    struct guest_virqmap *guest_virqmap;
};

void virq_create(struct virq *virq);
hvmm_status_t virq_init(struct virq *virq, vmid_t vmid);

uint32_t pirq_to_virq(struct virq *v, uint32_t pirq);
uint32_t virq_to_pirq(struct virq *v, uint32_t virq);
uint32_t pirq_to_enabled_virq(struct virq *v, uint32_t pirq);
uint32_t virq_to_enabled_pirq(struct virq *v, uint32_t virq);

void virq_enable(struct virq *v, uint32_t virq);

hvmm_status_t virq_save(struct virq *virq);
hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid);

#endif /* __VIRQ_H__ */
