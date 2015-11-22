
#include <stdio.h>

#include <scheduler_skeleton.h>
#include <scheduler.h>

/* TODO: Implement scheduler instance assignment to pCPU
 *  - const struct ptr array, something like 'assigned_sched'
 *  - need to be initialized
 *  - need some function like sched_set_policy(pcpu)
 *  - need to make general scheduler functions
 *      call policy functions through per-pCPU policy array
 */

/**
 * Initialize scheduler related data
 *
 * @param pcpuid ID of physical CPU
 * @param policy Policy to be used on pCPU
 * @return
 */
int sched_init()
{
    /* Check scheduler config */

    /* Allocate memory for system-wide data */

    /* Initialize data */
    // call sched_policy.init() for each policy implementation
    sched_rr.init();

    return 0;
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
int do_schedule()
{
    int next_vcpuid;

    /* get assigned scheduling policy of pCPU? */

    /* determine next vcpu to be run
     * by calling scheduler.do_schedule() */
    next_vcpuid = sched_rr.do_schedule();

    /* update vCPU's running time */


    /* manipulate variables to
     * cause context switch */
    printf("### NEXT vCPU: %d\n", next_vcpuid);

    return 0;
}
