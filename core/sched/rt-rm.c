#include <core/sched/scheduler_skeleton.h>
#include <core/sched/sched-config.h>
#include <config.h>
#include <arch/armv7.h>
#include <stdlib.h>
#include <debug.h>
#include <stdbool.h>

#include <lib/list.h>
#include <core/timer.h>

struct entry_data_rm {
    struct sched_entry *e;

    uint32_t period;
    uint32_t budget;

    uint32_t period_cntdn;
    uint32_t budget_cntdn;

    struct list_head head;
};

struct sched_data_rm {
    struct scheduler *s;

    uint64_t tick_interval_us;
    uint64_t tick_count;

    struct entry_data_rm *current;
    struct list_head runqueue;
};

/* Scheduler related data initialization */
void sched_rm_init(struct scheduler *s)
{
    struct sched_data_rm *sd = (struct sched_data_rm *) malloc(sizeof(struct sched_data_rm));

    /* Check scheduler config */
    /* Allocate memory for system-wide data */
    /* Initialize data */
    sd->current = NULL;
    sd->tick_count = 0;
    sd->tick_interval_us = schedconf_rm_tick_interval_ms[s->pcpuid];
    sd->s = s;

    LIST_INITHEAD(&sd->runqueue);

    s->sd = sd;
}

int sched_rm_vcpu_register(struct scheduler *s, struct sched_entry *e)
{
    // struct sched_data_rm *sd = (struct sched_data_rm *) (s->sd);
    struct entry_data_rm *ed = (struct entry_data_rm *) malloc(sizeof(struct entry_data_rm));

    ed->period = schedconf_rm_period_budget[e->vcpuid][0];
    ed->budget = schedconf_rm_period_budget[e->vcpuid][1];
    ed->e = e;

    LIST_INITHEAD(&ed->head);

    e->ed = ed;

    return 0;
}

int sched_rm_vcpu_unregister(struct scheduler *s, struct sched_entry *e)
{
    /* Check if vcpu is registered */
    /* Check if vcpu is detached. If not, request detachment.*/
    /* If we have requested detachment of vcpu,
     *   let's wait until it is detached by main scheduling routine */

    return 0;
}

int sched_rm_vcpu_attach(struct scheduler *s, struct sched_entry *e)
{
    struct sched_data_rm *sd = (struct sched_data_rm *) (s->sd);
    struct entry_data_rm *ed = (struct entry_data_rm *) (e->ed);

    ed->period_cntdn = ed->period;
    ed->budget_cntdn = ed->budget;

    LIST_ADDTAIL(&ed->head, &sd->runqueue);

    return 0;
}

int sched_rm_vcpu_detach(struct scheduler *s, struct sched_entry *e)
{
    // struct sched_data_rm *sd = (struct sched_data_rm *) (s->sd);
    struct entry_data_rm *ed = (struct entry_data_rm *) (e->ed);

    /* Check if vcpu is attached */
    /* Remove it from runqueue by setting will_detached flag*/
    /* Set entry_data_rm's fields */

    LIST_DELINIT(&ed->head);

    return 0;
}

/**
 * Main scheduler routine in RT - Rate Monotonic policy implementation
 */
int sched_rm_do_schedule(struct scheduler *s, uint64_t *expiration)
{
    struct sched_data_rm *sd = (struct sched_data_rm *) (s->sd);

    struct entry_data_rm *current_ed = NULL;
    struct entry_data_rm *next_ed = NULL;

    bool is_switching_needed = false;
    int next_vcpuid = VCPUID_INVALID;

    *expiration =
        timer_get_timenow() + USEC(sd->tick_interval_us);

    if (LIST_IS_EMPTY(&sd->runqueue)) {
        // printf("Nothing to run\n");
        // printf("Idle mode not implemented\n");
        while(1);
    }

    /* Check & decrease current running entry's budge count */
    if (sd->current == NULL) {
        is_switching_needed = true;
    } else {
        current_ed = sd->current;
        if (current_ed->budget_cntdn == 0) {
            is_switching_needed = true;
        } else {
            current_ed->budget_cntdn -= 1;
        }
    }

    /* Check & decrease all entries' period count */
    struct entry_data_rm *ed = NULL;
    if (is_switching_needed) {
        uint32_t min_period = 0xFFFFFFFF;
        next_ed = NULL;

        LIST_FOR_EACH_ENTRY(ed, &sd->runqueue, head) {
            if (ed->period_cntdn == 0) {
                ed->budget_cntdn = ed->budget;
                ed->period_cntdn = ed->period;
            }

            /* find the entry with budget left and shortest period */
            if (ed->budget_cntdn > 0 && min_period > ed->period) {
                next_ed = ed;
                min_period = ed->period;
            }

            ed->period_cntdn -= 1;
        }

        if (sd->current != NULL) {
            current_ed->e->state = SCHED_WAITING;
        }

        if (next_ed != NULL) {
            struct sched_entry *next_e = NULL;
            next_e = next_ed->e;
            next_e->state = SCHED_RUNNING;

            sd->current = next_ed;
            next_ed->budget_cntdn -= 1;

            next_vcpuid = next_e->vcpuid;
        } else {
            /* TODO:(igkang) handle the situation that there is no entry to run */
            // printf("Nothing to run\n");
            // printf("Idle mode not implemented\n");
            while(1);
        }
    } else {
        LIST_FOR_EACH_ENTRY(ed, &sd->runqueue, head) {
            if (ed->period_cntdn == 0) {
                ed->budget_cntdn = ed->budget;
                ed->period_cntdn = ed->period;
            }

            ed->period_cntdn -= 1;
        }

        if (sd->current != NULL) {
            next_vcpuid = current_ed->e->vcpuid;
        }
    }

    return next_vcpuid;
}

const struct sched_policy sched_rt_rm = {
    .init = sched_rm_init,
    .register_vcpu = sched_rm_vcpu_register,
    .unregister_vcpu = sched_rm_vcpu_unregister,
    .attach_vcpu = sched_rm_vcpu_attach,
    .detach_vcpu = sched_rm_vcpu_detach,
    .do_schedule = sched_rm_do_schedule
};
