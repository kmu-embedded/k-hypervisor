#include <stdio.h>
#include <debug_print.h>
#include <interrupt.h>
#include <rtsm-config.h>
#include <gic.h>
#include <vgic.h>

static struct vgic_status _vgic_status[NUM_GUESTS_STATIC];

static hvmm_status_t guest_interrupt_save(vmid_t vmid)
{
    return vgic_save_status(&_vgic_status[vmid]);
}

static hvmm_status_t guest_interrupt_restore(vmid_t vmid)
{
    return vgic_restore_status(&_vgic_status[vmid], vmid);
}
struct interrupt_ops _guest_interrupt_ops = {
    //.init = guest_interrupt_init,
    //.end = guest_interrupt_end,
    //.inject = guest_interrupt_inject,
    .save = guest_interrupt_save,
    .restore = guest_interrupt_restore,
    //.dump = guest_interrupt_dump,
};
