#include "vgic.h"
#include <stdio.h>

#include <hvmm_types.h>

#include <arch/armv7.h>
#include <arch/gic_regs.h>
#include <core/scheduler.h>
#include <debug_print.h>

#include <rtsm-config.h>
#include <core/vm/virq.h>
#include <io.h>
#include "gic-v2.h"

hvmm_status_t vgic_init_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        status->lr[i] = 0;
    }

    status->hcr = 0;
    status->apr = 0;
    status->vmcr = 0;
    status->saved_once = 0;

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_save_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        status->lr[i] = GICH_READ(GICH_LR(i));
    }

    status->hcr = GICH_READ(GICH_HCR);
    status->apr = GICH_READ(GICH_APR);
    status->vmcr = GICH_READ(GICH_VMCR);
    gich_enable(0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_restore_status(struct vgic_status *status, vcpuid_t vcpuid)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        GICH_WRITE(GICH_LR(i), status->lr[i]);
    }

    GICH_WRITE(GICH_APR, status->apr);
    GICH_WRITE(GICH_VMCR, status->vmcr);
    GICH_WRITE(GICH_HCR, status->hcr);
    /* Inject queued virqs to the next guest */
    /*
     * Staying at the currently active guest.
     * Flush out queued virqs since we didn't have a chance
     * to switch the context, where virq flush takes place,
     * this time
     */
    gic_inject_pending_irqs(vcpuid);
    gich_enable(1);

    return HVMM_STATUS_SUCCESS;
}

