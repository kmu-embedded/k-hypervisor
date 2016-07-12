#include <core/sched/scheduler_skeleton.h>
#include <config.h>
#include <arch/armv7.h>
#include <stdlib.h>
#include <debug.h>
#include <stdbool.h>

#include <lib/list.h>
#include <core/timer.h>

struct entry_data_rr {
    struct sched_entry *e;

    vcpuid_t vcpuid;
    uint32_t tick_reset_val;

    struct list_head head;
};

struct sched_data_rr {
    struct scheduler *s;

    struct list_head *current;
    struct list_head runqueue;
};

/**
 * Scheduler related data initialization
 */
void sched_rr_init(struct scheduler *s)
{
    struct sched_data_rr *sd = (struct sched_data_rr *) malloc(sizeof(struct sched_data_rr));

    /* Check scheduler config */
    /* Allocate memory for system-wide data */
    /* Initialize data */
    sd->current = NULL;
    sd->s = s;
    LIST_INITHEAD(&sd->runqueue);

    s->sd = sd;
}

int sched_rr_vcpu_register(struct scheduler *s, struct sched_entry *e)
{
    // struct sched_data_rr *sd = (struct sched_data_rr *) (s->sd);
    struct entry_data_rr *ed = (struct entry_data_rr *) malloc(sizeof(struct entry_data_rr));

    /* FIXME:(igkang) Hardcoded. should use function parameter's value for tick_reset_val init. */
    ed->tick_reset_val = 5;
    ed->e = e;

    LIST_INITHEAD(&ed->head);

    e->ed = ed;

    return 0;
}

int sched_rr_vcpu_unregister(struct scheduler *s, struct sched_entry *e)
{
    /* Check if vcpu is registered */
    /* Check if vcpu is detached. If not, request detachment.*/
    /* If we have requested detachment of vcpu,
     *   let's wait until it is detached by main scheduling routine */

    free(e->ed);
    return 0;
}

int sched_rr_vcpu_attach(struct scheduler *s, struct sched_entry *e)
{
    struct sched_data_rr *sd = (struct sched_data_rr *) (s->sd);
    struct entry_data_rr *ed = (struct entry_data_rr *) (e->ed);

    /* Add it to runqueue */
    LIST_ADDTAIL(&ed->head, &sd->runqueue);

    return 0;
}

int sched_rr_vcpu_detach(struct scheduler *s, struct sched_entry *e)
{
    // struct sched_data_rr *sd = (struct sched_data_rr *) (s->sd);
    struct entry_data_rr *ed = (struct entry_data_rr *) (e->ed);

    /* Check if vcpu is attached */
    /* Remove it from runqueue by setting will_detached flag*/
    /* Set entry_data_rr's fields */

    LIST_DELINIT(&ed->head);

    return 0;
}

/**
 * Main scheduler routine in RR policy implmentation
 */
int sched_rr_do_schedule(struct scheduler *s, uint64_t *expiration)
{
    struct sched_data_rr *sd = (struct sched_data_rr *) (s->sd);

    /* TODO:(igkang) change type to bool */
    struct entry_data_rr *next_ed = NULL;
    bool is_switching_needed = false;
    int next_vcpuid = VCPUID_INVALID;

    /* check pending attach list
     *      then attach them to runqueue */
    /* TODO:(igkang) write code to attach pending attach requests */

    /* TODO:(igkang) improve logical code structure to make it easier to read */

    /* determine next vcpu to be run
     *  - if there is an detach-pending vcpu than detach it. */
    if (sd->current == NULL) { /* No vCPU is running */
        if (!LIST_IS_EMPTY(&sd->runqueue)) { /* and there are some vcpus waiting */
            is_switching_needed = true;
        }
    } else { /* There's a vCPU currently running */
        struct entry_data_rr *current_ed = NULL;
        struct sched_entry *current_e = NULL;

        /* put current entry back to runqueue */
        current_ed = LIST_ENTRY(struct entry_data_rr, sd->current, head);
        LIST_ADDTAIL(sd->current, &sd->runqueue);

        /* let's switch as tick is over */
        sd->current = NULL;

        current_e = current_ed->e;
        current_e->state = SCHED_WAITING;

        is_switching_needed = true;
    }

    /* update scheduling-related data (like tick) */
    if (is_switching_needed) {
        /* move entry from runqueue to current */
        sd->current = sd->runqueue.next;
        LIST_DELINIT(sd->current);

        next_ed = LIST_ENTRY(struct entry_data_rr, sd->current, head);

        *expiration =
            timer_get_timenow() + MSEC(1) * (uint64_t) next_ed->tick_reset_val;
    }

    /* vcpu of current entry will be the next vcpu */
    if (sd->current != NULL) {
        struct sched_entry *next_e = NULL;
        next_ed = LIST_ENTRY(struct entry_data_rr, sd->current, head);

        next_e = next_ed->e;
        next_e->state = SCHED_RUNNING;

        /* set return next_vcpuid value */
        next_vcpuid = next_e->vcpuid;
    }

    return next_vcpuid;
}

/* TODO:(igkang) assign proper function's address to s-algo struct */
const struct sched_policy sched_rr = {
    .init = sched_rr_init,
    .register_vcpu = sched_rr_vcpu_register,
    .unregister_vcpu = sched_rr_vcpu_unregister,
    .attach_vcpu = sched_rr_vcpu_attach,
    .detach_vcpu = sched_rr_vcpu_detach,
    .do_schedule = sched_rr_do_schedule
};
