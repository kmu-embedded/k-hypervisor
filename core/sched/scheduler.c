#include <core/scheduler.h>
#include <core/sched/scheduler_skeleton.h>
#include <core/context_switch.h>
#include <core/interrupt.h>
#include <core/vm/vmem.h>
#include <core/vdev.h>
#include <arch/armv7.h>
#include <core/timer.h>
// TODO(wonseok): make it neat.
#include "../../arch/arm/vgic.h"
#include <stdio.h>
#include <stdlib.h>
#include <debug_print.h>
#include <hvmm_trace.h>

#include <rtsm-config.h>
#include <lib/bsd/list.h>

vcpuid_t __current_vcpuid[NR_CPUS];// = {VCPUID_INVALID, VCPUID_INVALID};
vcpuid_t __next_vcpuid[NR_CPUS];// = {VCPUID_INVALID, };
const struct scheduler *__policy[NR_CPUS];

/* TODO:(igkang) redesign runqueue & registered list structure for external external access */
struct list_head __running_vcpus[NR_CPUS];

/* TODO:(igkang) make sched functions run based on phisical CPU-assigned policy
 *   - [v] add pcpu sched mapping
 *   - [ ] modify functions parameters to use pCPU ID
 */

void sched_init()
{
    uint32_t pcpu = smp_processor_id();
    struct timer timer;

    /* Check scheduler config */

    /* Allocate memory for system-wide data */

    /* Initialize data */
    __current_vcpuid[pcpu] = VCPUID_INVALID;
    __next_vcpuid[pcpu] = VCPUID_INVALID;

    INIT_LIST_HEAD(&__running_vcpus[pcpu]);

    /* TODO:(igkang) choose policy based on config */
    __policy[pcpu] = &sched_rr;

    // call sched__policy.init() for each policy implementation
    __policy[pcpu]->init();
}

/* TODO:(igkang) context switching related fucntions should be redesigned
 *
 * in scheduler.c
 *   - perform_switch
 *   - perform_switch
 *   - sched__policy_determ_next
 *   - switchto
 *
 * and outside of scheduler.c
 *   - do_context_switch
 */
static hvmm_status_t perform_switch(struct core_regs *regs, vcpuid_t next_vcpuid)
{
    /* _curreng_vcpuid -> next_vcpuid */

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t pcpu = smp_processor_id();
    vcpuid_t currentcurrent = VCPUID_INVALID;

    if (__current_vcpuid[pcpu] == next_vcpuid)
        return HVMM_STATUS_IGNORED;

    currentcurrent = __current_vcpuid[pcpu];
    __current_vcpuid[pcpu] = next_vcpuid;
    do_context_switch(currentcurrent, next_vcpuid, regs);

    return result;
}

hvmm_status_t sched_perform_switch(struct core_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t pcpu = smp_processor_id();

    if (__current_vcpuid[pcpu] == VCPUID_INVALID) {
        /*
         * If the scheduler is not already running, launch default
         * first guest. It occur in initial time.
         */
        debug_print("context: launching the first guest\n");
        result = perform_switch(0, __next_vcpuid[pcpu]);
        /* DOES NOT COME BACK HERE */
    } else if (__next_vcpuid[pcpu] != VCPUID_INVALID && __current_vcpuid[pcpu] != __next_vcpuid[pcpu]) {
        debug_print("[sched] curr:%x next:%x\n", __current_vcpuid[pcpu], __next_vcpuid[pcpu]);
        /* Only if not from Hyp */
        result = perform_switch(regs, __next_vcpuid[pcpu]);
        __next_vcpuid[pcpu] = VCPUID_INVALID;
    }

    return result;
}

/* Switch to the first guest */
void sched_start(void)
{
    struct vcpu *vcpu = 0;
    uint32_t pcpu = smp_processor_id();
    struct timer timer;

    debug_print("[hyp] switch_to_initial_guest:\n");

    /* Select the first guest context to switch to. */
    vcpu = vcpu_find(__policy[pcpu]->do_schedule(&timer.interval));
    timer.callback = &do_schedule;
    timer_set(&timer, HOST_TIMER);

    switch_to(vcpu->vcpuid);
    sched_perform_switch(NULL);
}

vcpuid_t get_current_vcpuid(void)
{
    uint32_t pcpu = smp_processor_id();

    /* TODO:(igkang) let this function use API of policy implementation,
     *   instead of globally defined array */

    return __current_vcpuid[pcpu];
}

hvmm_status_t switch_to(vcpuid_t vcpuid)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t pcpu = smp_processor_id();

    /* TODO:(igkang) check about below comment */
    /* valid and not current vcpuid, switch */
    __next_vcpuid[pcpu] = vcpuid;
    result = HVMM_STATUS_SUCCESS;

    return result;
}

#if 0
vcpuid_t sched__policy_determ_next(void)
{
    uint32_t pcpu = smp_processor_id();

    /* FIXME:(igkang) Hardcoded for rr */
    vcpuid_t next = __policy[pcpu]->do_schedule();

    if (next == VCPUID_INVALID) {
        debug_print("policy_determ result: VCPUID_INVALID\n");
        next = 0;
        hyp_abort_infinite();
    }

    return next;
}
#endif

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
    uint32_t pcpu = smp_processor_id();
    /* call scheduler.register_vcpu() */
    __policy[pcpu]->register_vcpu(vcpuid);

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
int sched_vcpu_unregister(vcpuid_t vcpuid)
{
    uint32_t pcpu = smp_processor_id();
    /* call scheduler.unregister_vcpu() */
    __policy[pcpu]->unregister_vcpu(vcpuid);

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
int sched_vcpu_attach(vcpuid_t vcpuid)
{
    uint32_t pcpu = smp_processor_id();
    /* call scheduler.attach_vcpu() */
    __policy[pcpu]->attach_vcpu(vcpuid);

    return 0;
}

/**
 * Detach a vcpu from scheduler runqueue
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_detach(vcpuid_t vcpuid)
{
    uint32_t pcpu = smp_processor_id();
    /* call scheduler.detach_vcpu() */
    __policy[pcpu]->detach_vcpu(vcpuid);

    return 0;
}

/**
 * Main scheduler routine
 *
 * @param
 * @return
 */
void do_schedule(void *pdata, uint32_t *delay_tick)
{
    uint32_t pcpu = smp_processor_id();
    /* TODO:(igkang) function type(return/param) should be renewed */
    int next_vcpuid;

    /* get assigned scheduling policy of pCPU? */

    /* determine next vcpu to be run
     * by calling scheduler.do_schedule() */
    next_vcpuid = __policy[pcpu]->do_schedule(delay_tick);

    /* FIXME:(igkang) hardcoded */
    /* set timer for next scheduler work */
    // *delay_tick = 1 * TICKTIME_1MS / 50;

    /* update vCPU's running time */

    /* manipulate variables to
     * cause context switch */

    switch_to(next_vcpuid);
    sched_perform_switch(pdata);
}
