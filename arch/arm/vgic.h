#ifndef __VGIC_H__
#define __VGIC_H__

#include <stdint.h>
#include "gic-v2.h"

struct vgic_status {
    /* restore only if saved once to avoid dealing with corrupted data */
    uint32_t saved_once;
    uint32_t lr[64];        /**< List Registers */
    uint32_t hcr;           /**< Hypervisor Control Register */
    uint32_t apr;           /**< Active Priorities Register */
    uint32_t vmcr;          /**< Virtual Machine Control Register */
};

hvmm_status_t vgic_init_status(struct vgic_status *status);
hvmm_status_t vgic_save_status(struct vgic_status *status);
hvmm_status_t vgic_restore_status(struct vgic_status *status, vcpuid_t vcpuid);


#endif
