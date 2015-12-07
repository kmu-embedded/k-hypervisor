#include <arch/arm/rtsm-config.h>
#include <guest.h>
#include <timer.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>
#include <hvmm_trace.h>
#include <smp.h>
#include <stdio.h>
#include <scheduler.h>
#include <armv7_p15.h>
#include <vgic.h>

#define _valid_vmid(vmid) \
    (guest_first_vmid() <= vmid && guest_last_vmid() >= vmid)

hvmm_status_t guest_save(struct vcpu *guest, struct arch_regs *regs)
{
    /* guest_hw_save : save the current guest's context*/
    return  guest_hw_save(&guest->vcpu_regs, regs);
}

hvmm_status_t guest_restore(struct vcpu *guest, struct arch_regs *regs)
{
    /* guest_hw_restore : The next becomes the current */
     return  guest_hw_restore(&guest->vcpu_regs, regs);
}

void guest_dump_regs(struct arch_regs *regs)
{
    /* guest_hw_dump */
    guest_hw_dump(GUEST_VERBOSE_ALL, regs);
}

hvmm_status_t guest_init()
{
    struct timer_val timer;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    struct vcpu *guest;
    int i;
    int guest_count;
    int start_vmid = 0;
    uint32_t cpu = smp_processor_id();
    printf("[hyp] init_guests: enter\n");
    /* Initializes guests */
    guest_count = num_of_guest(cpu);

    if (cpu)
        start_vmid = num_of_guest(cpu - 1);
    else
        start_vmid = 0;

    guest_count += start_vmid;

    for (i = start_vmid; i < guest_count; i++) {
        /* Guest i @guest_bin_start */
        guest = &guests[i];
        guest->vmid = i;
        /* guest_hw_init */
        guest_hw_init(&guest->vcpu_regs);
    }

    printf("[hyp] init_guests: return\n");

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;

    result = timer_set(&timer, HOST_TIMER);

    if (result != HVMM_STATUS_SUCCESS)
        printf("[%s] timer startup failed...\n", __func__);

    return result;
}

void guest_copy(struct vcpu *dst, vmid_t vmid_src)
{
    guest_hw_move(&dst->vcpu_regs, &(guests[vmid_src]).vcpu_regs);
}

void reboot_guest(vmid_t vmid, uint32_t pc, struct arch_regs **regs)
{
    struct vcpu_regs *vcpu_regs = &guests[vmid].vcpu_regs;
    guest_hw_init(vcpu_regs);
    vcpu_regs->regs.pc = pc;
    vcpu_regs->regs.gpr[10] = 1;
    if (regs != 0)
        guest_hw_restore(vcpu_regs, *regs);
}

