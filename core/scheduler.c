#include <scheduler.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <timer.h>
#include <vgic.h>
#include <timer.h>

//extern struct vcpu running_vcpu[NUM_GUESTS_STATIC];
static int _current_guest_vmid[NUM_CPUS];// = {VMID_INVALID, VMID_INVALID};

static int _next_guest_vmid[NUM_CPUS];// = {VMID_INVALID, };
/* further switch request will be ignored if set */
static uint8_t _switch_locked[NUM_CPUS];

void sched_init()
{
    struct timer_val timer;

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model */
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;

    timer_set(&timer, HOST_TIMER);
}

static hvmm_status_t perform_switch(struct core_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    struct vcpu *guest = 0;
    uint32_t cpu = smp_processor_id();
    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED; /* the same guest? */

//    vcpu_save(&running_vcpu[_current_guest_vmid[cpu]], regs);
    vcpu_save(vcpu_find(_current_guest_vmid[cpu]), regs);
    memory_save();
    interrupt_save(_current_guest_vmid[cpu]);
    vdev_save(_current_guest_vmid[cpu]);

    /* The context of the next guest */
//    guest = &running_vcpu[next_vmid];
    guest = vcpu_find(next_vmid);
    _current_guest[cpu] = guest;
    _current_guest_vmid[cpu] = next_vmid;

    /* vcpu_regs_dump */
//    vcpu_regs_dump(GUEST_VERBOSE_LEVEL_3, &guest->vcpu_regs.core_regs);
//    print_core_regs(&guest->vcpu_regs.core_regs);

    vdev_restore(_current_guest_vmid[cpu]);

    interrupt_restore(_current_guest_vmid[cpu]);
    memory_restore(_current_guest_vmid[cpu]);
    vcpu_restore(guest, regs);

    return result;
}

hvmm_status_t guest_perform_switch(struct core_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    if (_current_guest_vmid[cpu] == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        printf("context: launching the first guest\n");

        result = perform_switch(0, _next_guest_vmid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid[cpu] != VMID_INVALID &&
                _current_guest_vmid[cpu] != _next_guest_vmid[cpu]) {
        printf("curr: %x\n", _current_guest_vmid[cpu]);
        printf("next: %x\n", _next_guest_vmid[cpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid[cpu]);
        _next_guest_vmid[cpu] = VMID_INVALID;
    }

    _switch_locked[cpu] = 0;
    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct vcpu *vcpu = 0;
    uint32_t cpu = smp_processor_id();

    printf("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid[cpu] = VMID_INVALID;
    if (cpu)
        vcpu = vcpu_find(2);
    else
        vcpu = vcpu_find(0);
    /* vcpu_regs_dump */
//    print_core_regs(&vcpu->vcpu_regs.core_regs);
//    vcpu_regs_dump(GUEST_VERBOSE_LEVEL_0, &vcpu->vcpu_regs.core_regs);
    /* Context Switch with current context == none */

    guest_switchto(vcpu->vcpuid, 0);
    guest_perform_switch(&vcpu->vcpu_regs.core_regs);
}

vmid_t guest_first_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 2;
    else
        return 0;
#endif
    return cpu;
}

vmid_t guest_last_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* FIXME:Hardcoded for now */
#if _SMP_
    if (cpu)
        return 3;
    else
        return 1;
#endif
    return cpu;
}

vmid_t guest_next_vmid(vmid_t ofvmid)
{
    vmid_t next = VMID_INVALID;
#if 0
#ifdef _SMP_
    uint32_t cpu = smp_processor_id();

    if (cpu)
        return 2;
    else
        return 0;
#endif
#endif

    /* FIXME:Hardcoded */
    if (ofvmid == VMID_INVALID)
        next = guest_first_vmid();
    else if (ofvmid < guest_last_vmid()) {
        /* FIXME:Hardcoded */
        next = ofvmid + 1;
    }
    return next;
}

vmid_t guest_current_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _current_guest_vmid[cpu];
}

vmid_t guest_waiting_vmid(void)
{
    uint32_t cpu = smp_processor_id();
    return _next_guest_vmid[cpu];
}


hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* valid and not current vmid, switch */
    if (_switch_locked[cpu] == 0) {
        _next_guest_vmid[cpu] = vmid;
        result = HVMM_STATUS_SUCCESS;
//        printf("switching to vmid: %x\n", (uint32_t)vmid);
    } else
        printf("context: next vmid locked to %d\n", _next_guest_vmid[cpu]);

    if (locked)
        _switch_locked[cpu] = locked;

    return result;
}

static int manually_next_vmid;
vmid_t selected_manually_next_vmid;
void set_manually_select_vmid(vmid_t vmid)
{
    manually_next_vmid = 1;
    selected_manually_next_vmid = vmid;
}
void clean_manually_select_vmid(void){
    manually_next_vmid = 0;
}

vmid_t sched_policy_determ_next(void)
{
#if 1
    if (manually_next_vmid)
        return selected_manually_next_vmid;

    vmid_t next = guest_next_vmid(guest_current_vmid());

    /* FIXME:Hardcoded */
    if (next == VMID_INVALID)
        next = guest_first_vmid();

    return next;
#endif
//    return guest_first_vmid();
}

void guest_schedule(void *pdata)
{
//    struct core_regs *regs = pdata;
    //uint32_t cpu = smp_processor_id();
    /* vcpu_regs_dump */
//    if (vcpu_regs_dump)
//    print_core_regs(regs);
//    vcpu_regs_dump(GUEST_VERBOSE_LEVEL_3, regs);
    /*
     * Note: As of guest_switchto() and guest_perform_switch()
     * are available, no need to test if trapped from Hyp mode.
     * guest_perform_switch() takes care of it
     */

    /* Switch request, actually performed at trap exit */
    guest_switchto(sched_policy_determ_next(), 0);

}
