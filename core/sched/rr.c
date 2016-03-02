#include <core/sched/scheduler_skeleton.h>
#include <lib/bsd/list.h>
#include <stdio.h>
#include <debug_print.h>
#include <stdbool.h>

typedef enum {
    DETACHED,
    WAITING,
    RUNNING
} state_rr;

struct rq_entry_rr {
    struct list_head head;
    struct list_head registered_list_head;

    /* TODO:(igkang) set field types to abstract types */
    int vcpuid; //vcpuid_t
    unsigned int tick; //tick_t
    unsigned int tick_reset_val; //tick_t
    state_rr state;
};

struct list_head *current = NULL;
struct list_head runqueue_rr;
struct list_head registered_list_rr;

/* TODO:(igkang) modify code to use dynamic allocation */
#define MAX_VCPUS 8 /* TODO:(igkang) Replace this macro with project-wide macro */
struct rq_entry_rr rq_entry_pool_rr[MAX_VCPUS];
int pool_front = 0;
int pool_rear = 0;


/* Function definitions goes here */
/* !! Funtions called by primary functions goes here !! */

struct rq_entry_rr *alloc_rq_entry_rr()
{
    if (pool_rear < MAX_VCPUS)
        return &rq_entry_pool_rr[pool_rear++];

    else
        return (struct rq_entry_rr *) NULL;
}

void print_all_entries_rr(void)
{
    struct rq_entry_rr *rq_entry;
    int cur_vcpuid = -1;

    /* print current running vCPU */
    if (current != NULL) {
        rq_entry = list_entry(current, struct rq_entry_rr, head);
        cur_vcpuid = rq_entry->vcpuid;
    }
    debug_print("RR CURRENT: %d\n", cur_vcpuid);

    /* print registered vCPU list */
    debug_print("RR REG_LIST Entries:");
    list_for_each_entry(rq_entry, &registered_list_rr, registered_list_head) {
        debug_print(" %d", rq_entry->vcpuid);
    }
    debug_print("\n");

    /* print attached vCPU list */
    debug_print("RR RUNQUEUE Entries:");
    list_for_each_entry(rq_entry, &runqueue_rr, head) {
        debug_print(" %d", rq_entry->vcpuid);
    }
    debug_print("\n");
}


/* !! Primary function impmentation goes here !! */

/**
 * Scheduler related data initialization
 *
 * @param
 * @return
 */
int sched_rr_init()
{
    /* Check scheduler config */

    /* Allocate memory for system-wide data */

    /* Initialize data */
    pool_front = 0;
    pool_rear = 0;

    current = NULL;
    INIT_LIST_HEAD(&runqueue_rr);
    INIT_LIST_HEAD(&registered_list_rr);

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
int sched_rr_vcpu_register(int vcpuid)
{
    struct rq_entry_rr *new_entry;

    /* Check if vcpu is already registered */

    /* Allocate a rq_entry_rr */
    new_entry = alloc_rq_entry_rr();

    /* Initialize rq_entry_rr instance */
    INIT_LIST_HEAD(&new_entry->registered_list_head);
    INIT_LIST_HEAD(&new_entry->head);

    /* FIXME:(igkang) should use function parameter's value for tick_reset_val init. */
    new_entry->vcpuid = vcpuid;
    new_entry->tick_reset_val = 5;
    new_entry->tick = new_entry->tick_reset_val;

    new_entry->state = DETACHED;

    /* Add it to registerd vcpus list */
    list_add_tail(&new_entry->registered_list_head, &registered_list_rr);

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
int sched_rr_vcpu_unregister()
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
int sched_rr_vcpu_attach(int vcpuid)
{
    struct rq_entry_rr *entry = NULL;
    struct rq_entry_rr *entry_to_be_attached = NULL;

    /* To find entry in registered entry list */
    list_for_each_entry(entry, &registered_list_rr, registered_list_head) {
        if (entry->vcpuid == vcpuid) {
            entry_to_be_attached = entry;
            break;
        }
    }

    /* TODO:(igkang) Name the return value constants. */
    if (entry_to_be_attached == NULL)
        return -1; /* error: not registered */

    if (entry_to_be_attached->state != DETACHED)
        return -2; /* error: already attached */

    /* Set rq_entry_rr's fields */
    entry_to_be_attached->state = WAITING;

    /* Add it to runqueue */
    list_add_tail(&entry_to_be_attached->head, &runqueue_rr);

    return 0;
}

/**
 *
 *
 * @param
 * @return
 */
int sched_rr_vcpu_detach()
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
int sched_rr_do_schedule()
{
    /* TODO:(igkang) change type to bool */
    struct rq_entry_rr *next_entry = NULL;
    bool is_switching_needed = false;
    int next_vcpuid = -1;

    /* check pending attach list
     *      then attach them to runqueue_rr */
    /* TODO:(igkang) write code to attach pending attach requests */

    /* TODO:(igkang) improve logical code structure to make it more readable */
    /* determine next vcpu to be run
     *  - if there is an detach-pending vcpu than detach it. */
    if (current == NULL) { /* No vCPU is running */
        if (!list_empty(&runqueue_rr)) /* and there are some vcpus waiting */
            is_switching_needed = true;
    } else { /* There's a vCPU currently running */
        struct rq_entry_rr *current_entry = NULL;

        /* check & decrease tick. if tick was <= 0 let's switch */
        current_entry = list_entry(current, struct rq_entry_rr, head);

        /* if tick is still left */
        if (current_entry->tick) {
            current_entry->tick--;
        } else { /* tick's over */
            is_switching_needed = true;

            /* reset tick for next scheduling */
            current_entry->tick = current_entry->tick_reset_val;

            /* put current entry back to runqueue_rr */
            current_entry->state = WAITING;
            list_add_tail(current, &runqueue_rr);
            current = NULL;
        }
    }

    /* update scheduling-related data (like tick) */
    if (is_switching_needed) {
        /* move entry from runqueue_rr to current */
        current = list_first(&runqueue_rr);
        list_del_init(current);

        next_entry = list_entry(current, struct rq_entry_rr, head);
        next_entry->tick -= 1;
    }

    /* vcpu of current entry will be the next vcpu */
    if (current != NULL) {
        next_entry = list_entry(current, struct rq_entry_rr, head);
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
