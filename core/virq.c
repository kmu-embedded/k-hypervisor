#include <virq.h>

void virq_setup()
{
    // guest_interrupt_init() ??
}

void virq_create()
{
    // Do we really need this function?
}

hvmm_status_t virq_init(struct virq *virq)
{
    /* TODO(igxactly): Needs initialization of guest_virqmap */

    return vgic_init_status(&virq->vgic_status);
}

hvmm_status_t virq_save(struct virq *virq)
{
//     hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* guest_interrupt_save() */
//     if (_guest_ops->save)
//         ret = _guest_ops->save(vmid);
//     return ret;

    return vgic_save_status(&virq->vgic_status);
}

hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid)
{
//     hvmm_status_t ret = HVMM_STATUS_UNKNOWN_ERROR;

    /* guest_interrupt_restore() */
//     if (_guest_ops->restore)
//         ret = _guest_ops->restore(vmid);
//     return ret;

    return vgic_restore_status(&virq->vgic_status, vmid);
}
