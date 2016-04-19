#include <core/sched/scheduler_skeleton.h>
#include <config.h>
#include <arch/armv7.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <stdbool.h>

#include <lib/list.h>
#include <core/timer.h>

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
int sched_rr_init(uint32_t pcpu)
{
    /* Check scheduler config */

    /* Allocate memory for system-wide data */

    /* Initialize data */
    current[pcpu] = NULL;

    LIST_INITHEAD(&runqueue_rr[pcpu]);
    LIST_INITHEAD(&registered_list_rr[pcpu]);

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
int sched_rr_vcpu_register(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct rq_entry_rr *new_entry;

    /* Check if vcpu is already registered */

    /* Allocate a rq_entry_rr */
    new_entry = (struct rq_entry_rr *) malloc(sizeof(struct rq_entry_rr));// alloc_rq_entry_rr();

    /* Initialize rq_entry_rr instance */
    LIST_INITHEAD(&new_entry->registered_list_head);
    LIST_INITHEAD(&new_entry->head);

    new_entry->vcpuid = vcpuid;

    /* FIXME:(igkang) Hardcoded. should use function parameter's value for tick_reset_val init. */
    new_entry->tick_reset_val = 5;

    new_entry->state = DETACHED;

    /* Add it to registerd vcpus list */
    LIST_ADDTAIL(&new_entry->registered_list_head, &registered_list_rr[pcpu]);

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
int sched_rr_vcpu_unregister(vcpuid_t vcpuid, uint32_t pcpu)
{
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
int sched_rr_vcpu_attach(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct rq_entry_rr *entry_to_be_attached = NULL;

    /* To find entry in registered entry list */
    struct rq_entry_rr *entry = NULL;
    list_for_each_entry(struct rq_entry_rr, entry, &registered_list_rr[pcpu], registered_list_head) {
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
    LIST_ADDTAIL(&entry_to_be_attached->head, &runqueue_rr[pcpu]);

    return 0;
}

/**
 *
 *
 * @param
 * @return
 */
int sched_rr_vcpu_detach(vcpuid_t vcpuid, uint32_t pcpu)
{
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
int sched_rr_do_schedule(uint64_t *expiration)
{
    uint32_t pcpu = smp_processor_id();
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
    if (current[pcpu] == NULL) { /* No vCPU is running */
        if (!LIST_IS_EMPTY(&runqueue_rr[pcpu])) { /* and there are some vcpus waiting */
            is_switching_needed = true;
        }
    } else { /* There's a vCPU currently running */
        struct rq_entry_rr *current_entry = NULL;
        /* put current entry back to runqueue_rr */
        current_entry = LIST_ENTRY(struct rq_entry_rr, current[pcpu], head);
        LIST_ADDTAIL(current[pcpu], &runqueue_rr[pcpu]);

        /* let's switch as tick is over */
        current_entry->state = WAITING;
        current[pcpu] = NULL;

        is_switching_needed = true;
    }

    /* update scheduling-related data (like tick) */
    if (is_switching_needed) {
        /* move entry from runqueue_rr to current */
        current[pcpu] = runqueue_rr[pcpu].next;
        LIST_DELINIT(current[pcpu]);

        next_entry = LIST_ENTRY(struct rq_entry_rr, current[pcpu], head);

        *expiration =
            timer_get_timenow() + MSEC(1) * (uint64_t) next_entry->tick_reset_val;
    }

    /* vcpu of current entry will be the next vcpu */
    if (current[pcpu] != NULL) {
//        printf("current[pcpu] != NULL : %d\n", LIST_ENTRY(struct rq_entry_rr, current[pcpu], head)->vcpuid);
        next_entry = LIST_ENTRY(struct rq_entry_rr, current[pcpu], head);
        next_entry->state = RUNNING;

        /* set return next_vcpuid value */
        next_vcpuid = next_entry->vcpuid;
//        printf("next_vcpuid : %d\n", next_vcpuid);
    }
//        printf("next_vcpuid : %d\n", next_vcpuid);

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
