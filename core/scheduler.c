#include <scheduler.h>
#include <sched/scheduler_skeleton.h>
#include <context_switch_to.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>
#include <armv7_p15.h>
#include <timer.h>
#include <vgic.h>
#include <timer.h>
#include <stdio.h>


//extern struct vcpu running_vcpu[NUM_GUESTS_STATIC];
int _current_guest_vmid[NUM_CPUS];// = {VMID_INVALID, VMID_INVALID};

int _next_guest_vmid[NUM_CPUS];// = {VMID_INVALID, };
/* further switch request will be ignored if set */
static uint8_t _switch_locked[NUM_CPUS];

void sched_init()
{
    struct timer_val timer;

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model */
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &do_schedule;

    timer_set(&timer, HOST_TIMER);

    /* Check scheduler config */
    /* Allocate memory for system-wide data */
    /* Initialize data */
    // call sched_policy.init() for each policy implementation
    sched_rr.init();
}

static hvmm_status_t perform_switch(struct core_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();
    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED;

    context_switch_to(_current_guest_vmid[cpu], next_vmid, regs);

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
    uint32_t cpu = 1;//smp_processor_id();

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
    } else
        printf("context: next vmid locked to %d\n", _next_guest_vmid[cpu]);

    if (locked)
        _switch_locked[cpu] = locked;

    return result;
}

vmid_t sched_policy_determ_next(void)
{
#if 1
    vmid_t next = guest_next_vmid(guest_current_vmid());

    /* FIXME:Hardcoded */
    if (next == VMID_INVALID)
        next = guest_first_vmid();

    return next;
#endif
}

/**
 * Register a vCPU to a scheduler
 *
 * You have to call sched_vcpu_attach() to \
 * run a vcpu by adding it to runqueue, additionally.
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @param pcpuid ID of physical CPU
 * @return
 */
int sched_vcpu_register(int vcpuid)
{
    /* call scheduler.register_vcpu() */
    sched_rr.register_vcpu(vcpuid);

    return 0;
}

/**
 * Unregister a vCPU from a scheduler
 *
 * You have to detach vcpu first.
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_unregister()
{
    /* call scheduler.unregister_vcpu() */
    sched_rr.unregister_vcpu();

    return 0;
}

/**
 * Attach a vcpu to scheduler runqueue
 *
 * You have to register the vcpu first
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_attach(int vcpuid)
{
    /* call scheduler.attach_vcpu() */
    sched_rr.attach_vcpu(vcpuid);

    return 0;
}

/**
 * Detach a vcpu from scheduler runqueue
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_detach()
{
    /* call scheduler.detach_vcpu() */
    sched_rr.detach_vcpu();

    return 0;
}

/**
 * Main scheduler routine
 *
 * @param
 * @return
 */
// int do_schedule()
void do_schedule(void *pdata)
{
    int next_vcpuid;

    /* get assigned scheduling policy of pCPU? */

    /* determine next vcpu to be run
     * by calling scheduler.do_schedule() */
    next_vcpuid = sched_rr.do_schedule();

    /* update vCPU's running time */

    /* manipulate variables to
     * cause context switch */

    guest_switchto(next_vcpuid, 0);
}
