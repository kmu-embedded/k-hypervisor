#include <scheduler.h>
#include <sched/scheduler_skeleton.h>
#include <context_switch_to.h>
#include <interrupt.h>
#include <vmem.h>
#include <vdev.h>
#include <armv7_p15.h>
#include <timer.h>
#include <vgic.h>
#include <timer.h>
#include <stdio.h>
#include <debug_print.h>
#include <hvmm_trace.h>

vcpuid_t _current_vcpuid[NUM_CPUS];// = {VMID_INVALID, VMID_INVALID};
vcpuid_t _next_vcpuid[NUM_CPUS];// = {VMID_INVALID, };

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
 *   - sched_perform_switch
 *   - sched_policy_determ_next
 *   - sched_switchto
 *
 * and outside of scheduler.c
 *   - context_switch_to
 */
static hvmm_status_t perform_switch(struct core_regs *regs, vcpuid_t next_vcpuid)
{
    /* _current_vcpuid -> next_vcpuid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();
    vcpuid_t current_vcpuid = VCPU_INVALID;

    if (_current_vcpuid[cpu] == next_vcpuid)
        return HVMM_STATUS_IGNORED;

    current_vcpuid = _current_vcpuid[cpu];
    _current_vcpuid[cpu] = next_vcpuid;
    context_switch_to(current_vcpuid, next_vcpuid, regs);

    return result;
}

hvmm_status_t sched_perform_switch(struct core_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    if (_current_vcpuid[cpu] == VMID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first vcpu. It occur in initial time.
         */
        debug_print("context: launching the first vcpu\n");
        result = perform_switch(0, _next_vcpuid[cpu]);
        /* DOES NOT COME BACK HERE */
    } else if (_next_vcpuid[cpu] != VMID_INVALID &&
            _current_vcpuid[cpu] != _next_vcpuid[cpu]) {
        debug_print("[sched] curr:%x next:%x\n", _current_vcpuid[cpu], _next_vcpuid[cpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, _next_vcpuid[cpu]);
        _next_vcpuid[cpu] = VMID_INVALID;
    }

    return result;
}

/* Switch to the first vcpu */
void sched_start(void)
{
    struct vcpu *vcpu = 0;
    uint32_t cpu = smp_processor_id();

    debug_print("[hyp] switch_to_initial_vcpu:\n");
    /* Select the first vcpu context to switch to. */
    _current_vcpuid[cpu] = VMID_INVALID;
    if (cpu)
        vcpu = vcpu_find(2);
    else
        vcpu = vcpu_find(0);

    sched_switchto(vcpu->vcpuid);
    sched_perform_switch(&vcpu->vcpu_regs.core_regs);
}

vcpuid_t get_current_vcpuid(void)
{
    uint32_t cpu = smp_processor_id();

    /* TODO:(igkang) let this function use API of policy implementation,
     *   instead of globally defined array */

    return _current_vcpuid[cpu];
}

hvmm_status_t sched_switchto(vcpuid_t vcpuid)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t cpu = smp_processor_id();

    /* TODO:(igkang) check about below comment */
    /* valid and not current vcpuid, switch */
    _next_vcpuid[cpu] = vcpuid;
    result = HVMM_STATUS_SUCCESS;

    return result;
}

vcpuid_t sched_policy_determ_next(void)
{
    /* FIXME:(igkang) Hardcoded for rr */
    vcpuid_t next = sched_rr.do_schedule();

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
int sched_vcpu_register(vcpuid_t vcpuid)
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

    sched_switchto(next_vcpuid);
}
