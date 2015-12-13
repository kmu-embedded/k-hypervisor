#include <stdio.h>
#include <interrupt.h>
#include <gic.h>
#include <vgic.h>

static struct vgic_status _vgic_status[NUM_GUESTS_STATIC];
//extern struct interrupt_ops _host_interrupt_ops;

static hvmm_status_t guest_interrupt_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* Virtual Interrupt: GIC Virtual Interface Control */
    result = vgic_init();
    if (result == HVMM_STATUS_SUCCESS)
        result = vgic_enable(1);

    virq_init();

    return result;
}

static hvmm_status_t guest_interrupt_end(uint32_t irq)
{
    return gic_completion_irq(irq);
}

static hvmm_status_t guest_interrupt_inject(vmid_t vmid, uint32_t virq,
                        uint32_t pirq, uint8_t hw)
{
    /* TODO : checking the injected bitmap */
    return virq_inject(vmid, virq, pirq, hw);
}

static hvmm_status_t guest_interrupt_save(vmid_t vmid)
{
    return vgic_save_status(&_vgic_status[vmid]);
}

static hvmm_status_t guest_interrupt_restore(vmid_t vmid)
{
    return vgic_restore_status(&_vgic_status[vmid], vmid);
}

static hvmm_status_t guest_interrupt_dump(void)
{
    /* TODO : dumpping the injected bitmap */
    return HVMM_STATUS_SUCCESS;
}

struct interrupt_ops _guest_interrupt_ops = {
    .init = guest_interrupt_init,
    .end = guest_interrupt_end,
    .inject = guest_interrupt_inject,
    .save = guest_interrupt_save,
    .restore = guest_interrupt_restore,
    .dump = guest_interrupt_dump,
};
