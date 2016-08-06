#include <config.h>
#include <arch/armv7.h>
#include <arch/irq.h>
#include <debug.h>
#include <core/timer.h>
#include <stdio.h>
#include <stdint.h>
#include <irq-chip.h>

#include <arch/armv7/generic_timer.h>

static struct list_head active_timers[NR_CPUS];
static struct list_head inactive_timers[NR_CPUS];
static struct timer_ops *__ops;

#ifdef CONFIG_TIMER_IRQ_STOPWATCH
struct stopwatch w_hyp[NR_CPUS];
struct stopwatch w_guest[NR_CPUS];
#endif /* CONFIG_TIMER_IRQ_STOPWATCH */

static hvmm_status_t timer_maintenance(void);

uint64_t timer_count_to_time_ns(uint64_t count)
{
    return count * TIMER_RESOLUTION_NS;
}

uint64_t timer_time_to_count_ns(uint64_t time)
{
    /* CNTFRQ > 10^9 ?? */
    return time / TIMER_RESOLUTION_NS;
}

static inline uint64_t get_syscounter(void)
{
    return __ops->get_counter();
}

uint64_t timer_get_syscounter(void)
{
    return get_syscounter();
}

uint64_t timer_get_timenow(void)
{
    return timer_count_to_time_ns(get_syscounter());
}

/*
 * Starts the timer.
 */
hvmm_status_t timer_start(void)
{
    /* timer_enable() */
    if (__ops->enable) {
        return __ops->enable();
    }

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 *  Stops the timer.
 */
hvmm_status_t timer_stop(void)
{
    /* timer_disable() */
    if (__ops->disable) {
        return __ops->disable();
    }

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

#ifdef CONFIG_TIMER_DEBUG
static uint64_t saved_syscnt[NR_CPUS] = {0,};
#endif

/*
 * This method handles all timer IRQ.
 */
// TODO(igkang): rename pregs and pdata for our usage.
static irqreturn_t timer_irq_handler(int irq, void *pregs, void *pdata)
{
    uint32_t pcpu = smp_processor_id();

#ifdef CONFIG_TIMER_IRQ_STOPWATCH
    if (w_hyp[pcpu].cnt > 0) {
        stopwatch_stop(&w_guest[pcpu]);
    }
    stopwatch_start(&w_hyp[pcpu]);
#endif /* CONFIG_TIMER_IRQ_STOPWATCH */

#ifdef CONFIG_TIMER_DEBUG
    uint64_t new_syscnt = get_syscounter();
    printf("time diff: %luns\n", (uint32_t)timer_count_to_time_ns(new_syscnt - saved_syscnt[pcpu]));
    saved_syscnt[pcpu] = new_syscnt;
#endif

    timer_stop();

    /* TODO:(igkang) serparate timer.c into two pieces and move one into arch/arm ? */
    uint64_t now = timer_count_to_time_ns(get_syscounter()); // * 10;

    struct timer *t;
    struct timer *tmp;
    LIST_FOR_EACH_ENTRY_SAFE(t, tmp, &active_timers[pcpu], head_active) {
        if (t->state == 1 && t->expiration < now) {
            t->callback(pregs, &t->expiration);

            t->state = (t->expiration == 0) ? 0 : 1;
            /* FIXME:(igkang) would be better to use set_timer instead? */
        }
    }

    timer_maintenance();

    timer_start();

#ifdef CONFIG_TIMER_IRQ_STOPWATCH
    stopwatch_stop(&w_hyp[pcpu]);
    stopwatch_start(&w_guest[pcpu]);

    if (w_hyp[pcpu].cnt >= 1000) {
        printf("w_hyp[%u]: cnt=%u min=%u max=%u total=%u \n", pcpu,
            w_hyp[pcpu].cnt, (uint32_t)w_hyp[pcpu].min,
            (uint32_t)w_hyp[pcpu].max, (uint32_t)w_hyp[pcpu].total);

        stopwatch_reset(&w_hyp[pcpu]);
    }

    if (w_guest[pcpu].cnt >= 1000) {
        printf("w_guest[%u]: cnt=%u min=%u max=%u total=%u \n", pcpu,
            w_guest[pcpu].cnt, (uint32_t)w_guest[pcpu].min,
            (uint32_t)w_guest[pcpu].max, (uint32_t)w_guest[pcpu].total);

        stopwatch_reset(&w_guest[pcpu]);
    }
#endif /* CONFIG_TIMER_IRQ_STOPWATCH */

    return VMM_IRQ;
}

static void timer_request_irq(uint32_t irq)
{
    register_irq_handler(irq, &timer_irq_handler, IRQ_LEVEL_SENSITIVE);
}

hvmm_status_t timer_hw_init(uint32_t irq) /* const struct timer_config const* timer_config)*/
{
    __ops = _timer_module.ops;

    /* FIXME:(igkang) may need per core initialization */
    if (__ops->init) {
        __ops->init();
    }

    /* TODO: (igkang) timer related call - check return value */
    timer_request_irq(irq);

    return HVMM_STATUS_SUCCESS;
}

/* non-percpu function. should be called only once, globally. */
hvmm_status_t timemanager_init() /* TODO: const struct timer_config const* timer_config)*/
{
    uint32_t pcpu;

    for (pcpu = 0; pcpu < NR_CPUS; pcpu++) {
        LIST_INITHEAD(&active_timers[pcpu]);
        LIST_INITHEAD(&inactive_timers[pcpu]);

#ifdef CONFIG_TIMER_IRQ_STOPWATCH
        stopwatch_init(&w_hyp[pcpu]);
        stopwatch_init(&w_guest[pcpu]);
#endif /* STOPWATCH_SCHEDULER */
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_register_timer(struct timer *t, timer_callback_t callback)
{
    uint32_t pcpu = smp_processor_id();
    /* ASSERT(t != NULL); */

    LIST_INITHEAD(&t->head_active);
    LIST_INITHEAD(&t->head_inactive);

    /* ASSERT(callback != NULL); */
    t->callback = callback;
    t->expiration = 0;
    t->state = 0; /* inactive */

    LIST_ADDTAIL(&t->head_inactive, &inactive_timers[pcpu]);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_set_timer(struct timer *t, uint64_t expiration, bool timer_stopstart)
{
    /* uint32_t pcpu = smp_processor_id(); */

    /* FIXME:(igkang) does it really need timer_stop/start ? */
    /* is the timer already disabled when we're calling this function? */
    if (timer_stopstart) {
        timer_stop();
    }

    /* ASSERT(t != NULL); */
    tm_deactivate_timer(t);

    /* ASSERT(expiration == 0); ? */
    t->expiration = expiration;
    tm_activate_timer(t);

    /* then do maintenance routine */
    timer_maintenance();

    if (timer_stopstart) {
        timer_start();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_activate_timer(struct timer *t)
{
    uint32_t pcpu = smp_processor_id();

    t->state = 1; /* active */

    /* ASSERT(t != NULL); */
    LIST_DELINIT(&t->head_inactive);
    LIST_ADDTAIL(&t->head_active, &active_timers[pcpu]);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_deactivate_timer(struct timer *t)
{
    uint32_t pcpu = smp_processor_id();

    t->state = 0; /* inactive */

    /* ASSERT(t != NULL); */
    LIST_DELINIT(&t->head_active);
    LIST_ADDTAIL(&t->head_inactive, &inactive_timers[pcpu]);

    return HVMM_STATUS_SUCCESS;
}

/* must be called between the calls of timer_stop() and timer_start() */
static hvmm_status_t timer_maintenance(void)
{
    uint32_t pcpu = smp_processor_id();

    /* then find nearest expiration time */
    uint64_t nearest = ~0x0llu;
    struct timer *t;
    LIST_FOR_EACH_ENTRY(t, &active_timers[pcpu], head_active) {
        if (t->expiration < nearest) {
            nearest = t->expiration;
        }
    }

    /* then calculate cval from ns */
    nearest = timer_time_to_count_ns(nearest);

    /* then set cval */
    /* ASSERT(__ops != NULL); */
    __ops->set_absolute(nearest);

    return HVMM_STATUS_SUCCESS;
}

/* cummulative stopwatch */
void stopwatch_init(struct stopwatch *w) {
    w->start = w->lastdiff = 0;
    stopwatch_reset(w);
}

void stopwatch_reset(struct stopwatch *w) {
    w->max = w->total = 0;
    w->min = 0xFFFFFFFFFFFFFFFF;
    w->cnt = 0;
}

void stopwatch_start(struct stopwatch *w) {
    w->start = get_syscounter();
    w->lastdiff = 0;
}

void stopwatch_stop(struct stopwatch *w) {
    w->lastdiff = get_syscounter() - w->start;

    if (w->lastdiff < w->min) {
        w->min = w->lastdiff;
    }

    if (w->lastdiff > w->max) {
        w->max = w->lastdiff;
    }

    w->total += w->lastdiff;
    w->cnt += 1;
}
