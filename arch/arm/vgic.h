#ifndef __VGIC_H__
#define __VGIC_H__

#include <stdint.h>
#include "gic-v2.h"


#define VGIC_NUM_MAX_SLOTS              64
#define VGIC_SLOT_NOTFOUND              (0xFFFFFFFF)

enum virq_state {
    VIRQ_STATE_INACTIVE = 0x00,
    VIRQ_STATE_PENDING = 0x01,
    VIRQ_STATE_ACTIVE = 0x02,
    VIRQ_STATE_PENDING_ACTIVE = 0x03,
};

struct vgic_status {
    /* restore only if saved once to avoid dealing with corrupted data */
    uint32_t saved_once;
    uint32_t lr[64];        /**< List Registers */
    uint32_t hcr;           /**< Hypervisor Control Register */
    uint32_t apr;           /**< Active Priorities Register */
    uint32_t vmcr;          /**< Virtual Machine Control Register */
};
hvmm_status_t vgic_enable(uint8_t enable);
hvmm_status_t vgic_init(void);

hvmm_status_t vgic_init_status(struct vgic_status *status);
hvmm_status_t vgic_save_status(struct vgic_status *status);
hvmm_status_t vgic_restore_status(struct vgic_status *status, vcpuid_t vcpuid);

hvmm_status_t vgic_flush_virqs(vcpuid_t vcpuid);
/* returns slot index if successful, VGIC_SLOT_NOTFOUND otherwise */
uint32_t vgic_inject_virq_sw(uint32_t virq, enum virq_state state,
                             uint32_t priority, uint32_t cpuid, uint8_t maintenance);
/* returns slot index if successful, VGIC_SLOT_NOTFOUND otherwise */
uint32_t vgic_inject_virq_hw(uint32_t virq, enum virq_state state,
                             uint32_t priority, uint32_t pirq);
uint32_t vgic_inject_virq(uint32_t virq, uint32_t slot, enum virq_state state,
                          uint32_t priority, uint8_t hw, uint32_t physrc,
                          uint8_t maintenance);
hvmm_status_t vgic_injection_enable(uint8_t enable);

hvmm_status_t virq_inject(vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw);
hvmm_status_t vgic_sgi(uint32_t cpu, enum gic_sgi sgi);

#endif
