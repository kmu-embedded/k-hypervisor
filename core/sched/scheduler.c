#include <core/scheduler.h>
#include <core/sched/scheduler_skeleton.h>
#include <core/context_switch_to.h>
#include <core/interrupt.h>
#include <core/vm/vmem.h>
#include <core/vdev.h>
#include <arch/armv7.h>
#include <core/timer.h>
// TODO(wonseok): make it neat.
#include "../../arch/arm/vgic.h"
#include <core/timer.h>

#include <stdio.h>
#include <debug_print.h>
#include <hvmm_trace.h>

#include <rtsm-config.h>

int _current_guest_vmid[NUM_CPUS];// = {VMID_INVALID, VMID_INVALID};
int _next_guest_vmid[NUM_CPUS];// = {VMID_INVALID, };

/* TODO:(igkang) rename sched functions
 *   - remove the word 'guest'
 */

/* TODO:(igkang) make sched functions run based on phisical CPU-assigned policy
 *
 *   - add pcpu sched mapping
 *   - modify functions parameters to use pCPU ID
 */

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

/* TODO:(igkang) context switching related fucntions should be redesigned
 *
 * in scheduler.c
 *   - perform_switch
 *   - guest_perform_switch
 *   - sched_policy_determ_next
 *   - guest_switchto
 *
 * and outside of scheduler.c
 *   - context_switch_to
 */
static hvmm_status_t perform_switch(struct core_regs *regs, vmid_t next_vmid)
{
    /* _curreng_guest_vmid -> next_vmid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();
    vcpuid_t currentcurrent = VCPU_INVALID;

    if (_current_guest_vmid[cpu] == next_vmid)
        return HVMM_STATUS_IGNORED;

    currentcurrent = _current_guest_vmid[cpu];
    _current_guest_vmid[cpu] = next_vmid;
    context_switch_to(currentcurrent, next_vmid, regs);

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
        debug_print("context: launching the first guest\n");
        result = perform_switch(0, _next_guest_vmid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_guest_vmid[cpu] != VMID_INVALID &&
            _current_guest_vmid[cpu] != _next_guest_vmid[cpu]) {
        debug_print("[sched] curr:%x next:%x\n", _current_guest_vmid[cpu], _next_guest_vmid[cpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_guest_vmid[cpu]);
        _next_guest_vmid[cpu] = VMID_INVALID;
    }

    return result;
}

/* Switch to the first guest */
void guest_sched_start(void)
{
    struct vcpu *vcpu = 0;
    uint32_t cpu = smp_processor_id();

    debug_print("[hyp] switch_to_initial_guest:\n");
    /* Select the first guest context to switch to. */
    _current_guest_vmid[cpu] = VMID_INVALID;
    if (cpu)
        vcpu = vcpu_find(2);
    else
        vcpu = vcpu_find(0);

    guest_switchto(vcpu->vcpuid);
    guest_perform_switch(&vcpu->vcpu_regs.core_regs);
}

vmid_t guest_current_vmid(void)
{
    uint32_t cpu = smp_processor_id();

    /* TODO:(igkang) let this function use API of policy implementation,
     *   instead of globally defined array */

    return _current_guest_vmid[cpu];
}

hvmm_status_t guest_switchto(vmid_t vmid)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* TODO:(igkang) check about below comment */
    /* valid and not current vmid, switch */
    _next_guest_vmid[cpu] = vmid;
    result = HVMM_STATUS_SUCCESS;

    return result;
}

vmid_t sched_policy_determ_next(void)
{
    /* FIXME:(igkang) Hardcoded for rr */
    vmid_t next = sched_rr.do_schedule();

    if (next == VMID_INVALID) {
        debug_print("policy_determ result: VMID_INVALID\n");
        next = 0;
        hyp_abort_infinite();
    }

    return next;
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
void do_schedule(void *pdata)
{
    /* TODO:(igkang) function type(return/param) should be renewed */
    int next_vcpuid;

    /* get assigned scheduling policy of pCPU? */

    /* determine next vcpu to be run
     * by calling scheduler.do_schedule() */
    next_vcpuid = sched_rr.do_schedule();

    /* update vCPU's running time */

    /* manipulate variables to
     * cause context switch */

    guest_switchto(next_vcpuid);
}
