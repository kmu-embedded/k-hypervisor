#include "vgic.h"
#include <arch/gic_regs.h>
#include <core/vm/virq.h>
#include <io.h>
//#include "gic-v2.h"

#include <irq-chip.h>
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

hvmm_status_t vgic_save_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < GICv2.num_lr; i++) {
        status->lr[i] = GICH_READ(GICH_LR(i));
    }

    status->hcr = GICH_READ(GICH_HCR);
    status->apr = GICH_READ(GICH_APR);
    status->vmcr = GICH_READ(GICH_VMCR);

    virq_hw->disable();

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
    virq_hw->forward_pending_irq(vcpuid);


    virq_hw->disable();

    return HVMM_STATUS_SUCCESS;
}

