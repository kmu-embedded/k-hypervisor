#include <core/sched/scheduler_skeleton.h>
#include <config.h>
#include <arch/armv7.h>
#include <stdlib.h>
#include <debug.h>
#include <stdbool.h>

#include <lib/list.h>
#include <core/timer.h>

struct rq_entry_rr {
    struct list_head head;

    /* TODO:(igkang) set field types to abstract types */
    vcpuid_t vcpuid;
    uint32_t tick_reset_val; //tick_t
};

struct sched_rr_data {
    struct list_head *current;
    struct list_head runqueue;
};

/**
 * Scheduler related data initialization
 */
void sched_rr_init(struct scheduler *sched)
{
    struct sched_rr_data *d = (struct sched_rr_data *) (sched + 1);

    /* Check scheduler config */
    /* Allocate memory for system-wide data */
    /* Initialize data */
    d->current = NULL;
    LIST_INITHEAD(&d->runqueue);
}

int sched_rr_vcpu_register(struct scheduler *sched, struct sched_entry *e)
{
    // struct sched_rr_data *d = (struct sched_rr_data *) (sched + 1);
    struct rq_entry_rr *ed = (struct rq_entry_rr *) (e + 1);

    /* FIXME:(igkang) Hardcoded. should use function parameter's value for tick_reset_val init. */
    ed->tick_reset_val = 5;
    LIST_INITHEAD(&ed->head);

    return 0;
}

int sched_rr_vcpu_unregister(struct scheduler *sched, struct sched_entry *e)
{
    /* Check if vcpu is registered */
    /* Check if vcpu is detached. If not, request detachment.*/
    /* If we have requested detachment of vcpu,
     *   let's wait until it is detached by main scheduling routine */

    return 0;
}

int sched_rr_vcpu_attach(struct scheduler *sched, struct sched_entry *e)
{
    struct sched_rr_data *d = (struct sched_rr_data *) (sched + 1);
    struct rq_entry_rr *ed = (struct rq_entry_rr *) (e + 1);

    /* Add it to runqueue */
    LIST_ADDTAIL(&ed->head, &d->runqueue);

    return 0;
}

int sched_rr_vcpu_detach(struct scheduler *sched, struct sched_entry *e)
{
    // struct sched_rr_data *d = (struct sched_rr_data *) (sched + 1);
    struct rq_entry_rr *ed = (struct rq_entry_rr *) (e + 1);

    /* Check if vcpu is attached */
    /* Remove it from runqueue by setting will_detached flag*/
    /* Set rq_entry_rr's fields */

    LIST_DELINIT(&ed->head);

    return 0;
}

/**
 * Main scheduler routine in RR policy implmentation
 */
int sched_rr_do_schedule(struct scheduler *sched, uint64_t *expiration)
{
    struct sched_rr_data *d = (struct sched_rr_data *) (sched + 1);

    /* TODO:(igkang) change type to bool */
    struct rq_entry_rr *next_rr_entry = NULL;
    bool is_switching_needed = false;
    int next_vcpuid = VCPUID_INVALID;

    /* check pending attach list
     *      then attach them to runqueue */
    /* TODO:(igkang) write code to attach pending attach requests */

    /* TODO:(igkang) improve logical code structure to make it easier to read */

    /* determine next vcpu to be run
     *  - if there is an detach-pending vcpu than detach it. */
    if (d->current == NULL) { /* No vCPU is running */
        if (!LIST_IS_EMPTY(&d->runqueue)) { /* and there are some vcpus waiting */
            is_switching_needed = true;
        }
    } else { /* There's a vCPU currently running */
        struct rq_entry_rr *current_rr_entry = NULL;
        struct sched_entry *current_entry = NULL;

        /* put current entry back to runqueue */
        current_rr_entry = LIST_ENTRY(struct rq_entry_rr, d->current, head);
        LIST_ADDTAIL(d->current, &d->runqueue);

        /* let's switch as tick is over */
        d->current = NULL;

        current_entry = ((struct sched_entry *) current_rr_entry) - 1;
        current_entry->state = SCHED_WAITING;

        is_switching_needed = true;
    }

    /* update scheduling-related data (like tick) */
    if (is_switching_needed) {
        /* move entry from runqueue to current */
        d->current = d->runqueue.next;
        LIST_DELINIT(d->current);

        next_rr_entry = LIST_ENTRY(struct rq_entry_rr, d->current, head);

        *expiration =
            timer_get_timenow() + MSEC(1) * (uint64_t) next_rr_entry->tick_reset_val;
    }

    /* vcpu of current entry will be the next vcpu */
    if (d->current != NULL) {
        struct sched_entry *next_entry = NULL;
        next_rr_entry = LIST_ENTRY(struct rq_entry_rr, d->current, head);

        next_entry = ((struct sched_entry *) next_rr_entry) - 1;
        next_entry->state = SCHED_RUNNING;

        /* set return next_vcpuid value */
        next_vcpuid = next_entry->vcpuid;
    }

    return next_vcpuid;
}

/* TODO:(igkang) assign proper function's address to sched-algo struct */
const struct sched_policy sched_rr = {
    .size_sched_extra = sizeof(struct sched_rr_data),
    .size_entry_extra = sizeof(struct rq_entry_rr),

    .init = sched_rr_init,
    .register_vcpu = sched_rr_vcpu_register,
    .unregister_vcpu = sched_rr_vcpu_unregister,
    .attach_vcpu = sched_rr_vcpu_attach,
    .detach_vcpu = sched_rr_vcpu_detach,
    .do_schedule = sched_rr_do_schedule
};
