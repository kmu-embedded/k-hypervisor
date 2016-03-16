#ifndef __VIRQ_H__
#define __VIRQ_H__

#include <hvmm_types.h>
#include "../../../core/vm/vgic.h"
#include <core/irq.h>

#define MAX_PENDING_VIRQS    128
#define MAX_NR_IRQ           1024

struct virq_entry {
    uint32_t pirq;
    uint32_t virq;
    uint8_t hw;
    uint8_t valid;
};

struct virqmap_entry {
    uint32_t enabled;
    uint32_t virq;
    uint32_t pirq;
};

struct guest_virqmap {
    struct virqmap_entry map[MAX_NR_IRQ];
};

struct virq {
    struct vgic_status vgic_status;
    struct guest_virqmap *guest_virqmap;
    struct virq_entry pending_irqs[MAX_PENDING_VIRQS +1];
};

void virq_create(struct virq *virq);
void virq_init(struct virq *virq, vmid_t vmid);

uint32_t pirq_to_virq(struct virq *v, uint32_t pirq);
uint32_t virq_to_pirq(struct virq *v, uint32_t virq);
uint32_t pirq_to_enabled_virq(struct virq *v, uint32_t pirq);
uint32_t virq_to_enabled_pirq(struct virq *v, uint32_t virq);

void virq_enable(struct virq *v, uint32_t virq);

hvmm_status_t virq_save(struct virq *virq);
hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid);

#endif /* __VIRQ_H__ */
