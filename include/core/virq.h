#ifndef __VIRQ_H__
#define __VIRQ_H__

#include <hvmm_types.h>
#include <stdint.h>
#include <vgic.h>
#include <interrupt.h>
#include <arch/arm/rtsm-config.h>

struct virq {
    struct vgic_status vgic_status;
    struct guest_virqmap *guest_virqmap;
};

void virq_setup();
void virq_create(struct virq *virq, vmid_t vmid);

hvmm_status_t virq_init(struct virq *virq, vmid_t vmid);
hvmm_status_t virq_save(struct virq *virq);
hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid);

#endif /* __VIRQ_H__ */
