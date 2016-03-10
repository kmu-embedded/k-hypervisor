#include <core/sched/scheduler_skeleton.h>
#include <rtsm-config.h>
#include <arch/armv7.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug_print.h>
#include <stdbool.h>

#ifdef CONFIG_C99
#include <lib/util_list.h>
#else
#include <lib/gnu/list.h>
#endif

typedef enum {
    DETACHED,
    WAITING,
    RUNNING
} state_rr;

struct rq_entry_rr {
    struct list_head head;
    struct list_head registered_list_head;

    /* TODO:(igkang) set field types to abstract types */
    vcpuid_t vcpuid;
    uint32_t tick_reset_val; //tick_t
    state_rr state;
};

struct list_head *current[NR_CPUS];
struct list_head runqueue_rr[NR_CPUS];
struct list_head registered_list_rr[NR_CPUS];

/* Function definitions goes here */

/**
 * Scheduler related data initialization
 *
 * @param
 * @return
 */
int sched_rr_init()
{
    uint32_t cpu = smp_processor_id();
    /* Check scheduler config */

    /* Allocate memory for system-wide data */

    /* Initialize data */
    current[cpu] = NULL;
    INIT_LIST_HEAD(&runqueue_rr[cpu]);
    INIT_LIST_HEAD(&registered_list_rr[cpu]);

    return 0;
}

/**
 * Register a vCPU to a scheduler
 *
 * You have to call sched_vcpu_attach() to \
 * run a vcpu by adding it to runqueue, additionally.
 *
 * @param shed A scheduler definition
 * @param vcpu A vCPU
 * @return
 */
int sched_rr_vcpu_register(vcpuid_t vcpuid)
{
    uint32_t cpu = smp_processor_id();
    struct rq_entry_rr *new_entry;

    /* Check if vcpu is already registered */

    /* Allocate a rq_entry_rr */
    new_entry = (struct rq_entry_rr *) malloc(sizeof(struct rq_entry_rr));// alloc_rq_entry_rr();

    /* Initialize rq_entry_rr instance */
    INIT_LIST_HEAD(&new_entry->registered_list_head);
    INIT_LIST_HEAD(&new_entry->head);

    new_entry->vcpuid = vcpuid;

    /* FIXME:(igkang) Hardcoded. should use function parameter's value for tick_reset_val init. */
    new_entry->tick_reset_val = 5;

    new_entry->state = DETACHED;

    /* Add it to registerd vcpus list */
    list_add_tail(&new_entry->registered_list_head, &registered_list_rr[cpu]);

    return 0;
}

/**
 * Unregister a vCPU from a scheduler
 *
 * Better NOT to use vcpu_unregister until \
 * dynamic allocation is applied
 *
 * @param shed A scheduler definition
 * @param vcpu A vCPU
 * @return
 */
int sched_rr_vcpu_unregister(vcpuid_t vcpuid)
{
    uint32_t cpu = smp_processor_id();
    /* Check if vcpu is registered */

    /* Check if vcpu is detached. If not, request detachment.*/

    /* If we have requested detachment of vcpu,
     *   let's wait until it is detached by main scheduling routine */

    /* Remove it from registered list */

    /* Deallocate rq_entry_rr */
    /* FIXME:(igkang) Deallocation will cause problem as  we are using
     *   array-base pool for now. Dynamic allocation fucntion
     *   is needed. Better NOT to use vcpu_unregister until
     *   this problem is fixed*/

    return 0;
}

/**
 *
 *
 * @param
 * @return
 */
int sched_rr_vcpu_attach(vcpuid_t vcpuid)
{
    uint32_t cpu = smp_processor_id();
    struct rq_entry_rr *entry = NULL;
    struct rq_entry_rr *entry_to_be_attached = NULL;

    /* To find entry in registered entry list */
    list_for_each_entry(entry, &registered_list_rr[cpu], registered_list_head) {
        if (entry->vcpuid == vcpuid) {
            entry_to_be_attached = entry;
            break;
        }
    }

    /* TODO:(igkang) Name the return value constants. */
    if (entry_to_be_attached == NULL) {
        return -1;    /* error: not registered */
    }

    if (entry_to_be_attached->state != DETACHED) {
        return -2;    /* error: already attached */
    }

    /* Set rq_entry_rr's fields */
    entry_to_be_attached->state = WAITING;

    /* Add it to runqueue */
    list_add_tail(&entry_to_be_attached->head, &runqueue_rr[cpu]);

    return 0;
}

/**
 *
 *
 * @param
 * @return
 */
int sched_rr_vcpu_detach(vcpuid_t vcpuid)
{
    uint32_t cpu = smp_processor_id();
    /* Check if vcpu is attached */

    /* Remove it from runqueue by setting will_detached flag*/

    /* Set rq_entry_rr's fields */

    return 0;
}

/**
 * Main scheduler routine in RR policy implmentation
 *
 * @param
 * @return next_vcpuid
 */
int sched_rr_do_schedule(uint32_t *delay_tick)
{
    uint32_t cpu = smp_processor_id();
    /* TODO:(igkang) change type to bool */
    struct rq_entry_rr *next_entry = NULL;
    bool is_switching_needed = false;
    int next_vcpuid = -1;

    /* check pending attach list
     *      then attach them to runqueue_rr */
    /* TODO:(igkang) write code to attach pending attach requests */

    /* TODO:(igkang) improve logical code structure to make it easier to read */
    /* determine next vcpu to be run
     *  - if there is an detach-pending vcpu than detach it. */
    if (current[cpu] == NULL) { /* No vCPU is running */
        if (!list_empty(&runqueue_rr[cpu])) { /* and there are some vcpus waiting */
            is_switching_needed = true;
        }
    } else { /* There's a vCPU currently running */
        struct rq_entry_rr *current_entry = NULL;

        /* check & decrease tick. if tick was <= 0 let's switch */
        current_entry = list_entry(current[cpu], struct rq_entry_rr, head);

        /* tick's over */
        is_switching_needed = true;

        /* put current entry back to runqueue_rr */
        current_entry->state = WAITING;
        list_add_tail(current[cpu], &runqueue_rr[cpu]);
        current[cpu] = NULL;
    }

    /* update scheduling-related data (like tick) */
    if (is_switching_needed) {
        /* move entry from runqueue_rr to current */
        current[cpu] = list_first(&runqueue_rr[cpu]);
        list_del_init(current[cpu]);

        next_entry = list_entry(current[cpu], struct rq_entry_rr, head);

        /* FIXME:(igkang) hardcoded */
        *delay_tick = next_entry->tick_reset_val * TICKTIME_1MS;
    }

    /* vcpu of current entry will be the next vcpu */
    if (current[cpu] != NULL) {
        next_entry = list_entry(current[cpu], struct rq_entry_rr, head);
        next_entry->state = RUNNING;

        /* set return next_vcpuid value */
        next_vcpuid = next_entry->vcpuid;
    }

    return next_vcpuid;
}

/* TODO:(igkang) assign proper function's address to sched-algo struct */
const struct scheduler sched_rr = {
    .init = sched_rr_init,
    .register_vcpu = sched_rr_vcpu_register,
    .unregister_vcpu = sched_rr_vcpu_unregister,
    .attach_vcpu = sched_rr_vcpu_attach,
    .detach_vcpu = sched_rr_vcpu_detach,
    .do_schedule = sched_rr_do_schedule
};
