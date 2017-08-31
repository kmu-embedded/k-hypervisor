//#include <core/sched/scheduler_skeleton.h>
#include <core/sched/sched-config.h>
#include <config.h>
#include <arch/armv7.h>
#include <stdlib.h>
#include <debug.h>
#include <stdbool.h>

#include <lib/list.h>
#include <core/timer.h>

struct entry_data_edf {
    struct sched_entry *e;

    uint32_t period;
    uint32_t budget;

    uint32_t period_cntdn;
    uint32_t budget_cntdn;

#ifdef CONFIG_SCHED_EDF_REPORT
    /* for analysis */
    uint32_t running_ticks;
    uint32_t preempted;
#endif

    struct list_head head;
};

struct sched_data_edf {
    struct scheduler *s;

    uint64_t tick_interval_us;
    uint64_t ticks;
#ifdef CONFIG_SCHED_EDF_REPORT
    uint64_t count_1k_ticks;
#endif

    struct entry_data_edf *current;
    struct list_head runqueue;
};

/* Scheduler related data initialization */
void sched_edf_init(struct scheduler *s)
{
    /* Allocate memory for system-wide data */
    struct sched_data_edf *sd = (struct sched_data_edf *) malloc(sizeof(struct sched_data_edf));

    /* Initialize data according to config */
    sd->current = NULL;
    sd->ticks = 0;
#ifdef CONFIG_SCHED_EDF_REPORT
    sd->count_1k_ticks = 1000;
#endif
    sd->tick_interval_us = schedconf_edf_tick_interval_us[s->pcpuid];
    sd->s = s;

    LIST_INITHEAD(&sd->runqueue);

    s->sd = sd;
}

int sched_edf_vcpu_register(struct scheduler *s, struct sched_entry *e)
{
    struct entry_data_edf *ed = (struct entry_data_edf *) malloc(sizeof(struct entry_data_edf));

    ed->period = schedconf_edf_period_budget[e->vcpuid][0];
    ed->budget = schedconf_edf_period_budget[e->vcpuid][1];
    ed->e = e;

#ifdef CONFIG_SCHED_EDF_REPORT
    ed->running_ticks = 0;
    ed->preempted = 0;
#endif

    LIST_INITHEAD(&ed->head);

    e->ed = ed;

    return 0;
}

int sched_edf_vcpu_unregister(struct scheduler *s, struct sched_entry *e)
{
    /* TODO:(igkang) Finish writing EDF vCPU unregister function */
    /* Check if vcpu is registered */
    /* Check if vcpu is detached. If not, request detachment.*/
    /* If we have requested detachment of vcpu,
     *   wait until it is detached by main scheduling routine */

    return 0;
}

int sched_edf_vcpu_attach(struct scheduler *s, struct sched_entry *e)
{
    struct sched_data_edf *sd = (struct sched_data_edf *) (s->sd);
    struct entry_data_edf *ed = (struct entry_data_edf *) (e->ed);

    ed->period_cntdn = ed->period;
    ed->budget_cntdn = ed->budget;

    LIST_ADDTAIL(&ed->head, &sd->runqueue);

    return 0;
}

int sched_edf_vcpu_detach(struct scheduler *s, struct sched_entry *e)
{
    struct entry_data_edf *ed = (struct entry_data_edf *) (e->ed);

    /* TODO:(igkang) Finish writing EDF vCPU detach function */
    /* Check if vcpu is attached */
    /* Remove it from runqueue by setting will_detached flag*/
    /* Set entry_data_edf's fields */

    LIST_DELINIT(&ed->head);

    return 0;
}

/**
 * Main scheduler routine in RT - Rate Monotonic policy implementation
 */
int sched_edf_do_schedule(struct scheduler *s, uint64_t *expiration)
{
    struct sched_data_edf *sd = (struct sched_data_edf *) (s->sd);

    struct entry_data_edf *current_ed = NULL;
    struct entry_data_edf *next_ed = NULL;

    int next_vcpuid = VCPUID_INVALID;

    sd->ticks += 1;

    if (LIST_IS_EMPTY(&sd->runqueue)) {
        printf("Nothing to run\n");
        while (1);
    }

    /* Check & decrease all entries' period count */
    struct entry_data_edf *ed = NULL;
    uint32_t min_period = 0xFFFFFFFF;
    next_ed = NULL;

    LIST_FOR_EACH_ENTRY(ed, &sd->runqueue, head) {
        if (ed->period_cntdn == 0) {
            if (ed->budget_cntdn != 0) {
                printf("Deadline miss on vCPU %u !", ed->e->vcpuid);
            }

            ed->budget_cntdn = ed->budget;
            ed->period_cntdn = ed->period;
        }

        /* find the entry with budget left and shortest period left */
        if (ed->budget_cntdn > 0 && min_period > ed->period_cntdn) {
            next_ed = ed;
            min_period = ed->period;
        }

        ed->period_cntdn -= 1;
    }

    if (sd->current != NULL) {
        current_ed = sd->current;

        if (current_ed->budget_cntdn == 0) { // out of budget
            current_ed->e->state = SCHED_WAITING;
        } else if (current_ed->period_cntdn > next_ed->period_cntdn) { // preemption!
#ifdef CONFIG_SCHED_EDF_REPORT
            /* current_ed->e->vcpuid is preempted by next_ed->e->vcpuid */
            current_ed->preempted += 1;
#endif
        } else { // continue to run
            next_ed = current_ed;
        }
    }

    if (next_ed != NULL) {
        struct sched_entry *next_e = NULL;
        next_e = next_ed->e;
        next_e->state = SCHED_RUNNING;

        sd->current = next_ed;
        next_ed->budget_cntdn -= 1;

#ifdef CONFIG_SCHED_EDF_REPORT
        next_ed->running_ticks += 1;
#endif

        next_vcpuid = next_e->vcpuid;
    } else {
        /* Never reach here */
        /* We use idle guest which does nothing when it is running */
        printf("Idle guest is needed!\n");
        while (1);
    }

#ifdef CONFIG_SCHED_EDF_REPORT
    if (sd->ticks >= sd->count_1k_ticks) {
        printf("EDF report (for %u ticks):\n", sd->tick_interval_us);

        LIST_FOR_EACH_ENTRY(ed, &sd->runqueue, head) {
            printf("    vcpu%u: running=%u preempted=%u\n", ed->e->vcpuid, ed->running_ticks, ed->preempted);
            ed->running_ticks = 0;
            ed->preempted = 0;
        }

        sd->count_1k_ticks += 1000;
    }
#endif

    if (*expiration == 0) {
        *expiration = timer_get_timenow();
    }

    *expiration = *expiration + USEC(sd->tick_interval_us);

    return next_vcpuid;
}

const struct sched_policy sched_rt_edf = {
    .init = sched_edf_init,
    .register_vcpu = sched_edf_vcpu_register,
    .unregister_vcpu = sched_edf_vcpu_unregister,
    .attach_vcpu = sched_edf_vcpu_attach,
    .detach_vcpu = sched_edf_vcpu_detach,
    .do_schedule = sched_edf_do_schedule
};
