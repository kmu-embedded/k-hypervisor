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

static hvmm_status_t timer_maintenance(void);

/* TODO:(igkang) Let definitions of time unit conversion functions be available
 *  conditionally by config macro variables (if < 0 then don't define)
 */

/*
 * Converts time unit from/to microseconds to/from system counter count.
 */
static inline uint64_t us_to_count(uint64_t time_in_us)
{
    return time_in_us * COUNT_PER_USEC;
}

static inline uint64_t ten_ns_to_count(uint64_t time_in_ten_ns)
{
    return time_in_ten_ns * (COUNT_PER_USEC / 100);
}

static inline uint64_t ms_to_count(uint64_t time_in_ms)
{
    return time_in_ms * (COUNT_PER_USEC * 1000);
}

static inline uint64_t sec_to_count(uint64_t time_in_sec)
{
    return time_in_sec * (COUNT_PER_USEC * 1000 * 1000);
}

static inline uint64_t count_to_us(uint64_t count)
{
    return count / COUNT_PER_USEC;
}

static inline uint64_t count_to_ten_ns(uint64_t count)
{
    return count / (COUNT_PER_USEC / 100);
}

static inline uint64_t count_to_ms(uint64_t count)
{
    return count / (COUNT_PER_USEC * 1000);
}

static inline uint64_t count_to_sec(uint64_t count)
{
    return count / (COUNT_PER_USEC * 1000 * 1000);
}

uint64_t timer_time_to_count(uint64_t time, time_unit_t unit)
{
    switch (unit) {
    case TIMEUNIT_USEC:
        return us_to_count(time);
    case TIMEUNIT_10NSEC:
        return ten_ns_to_count(time);
    case TIMEUNIT_MSEC:
        return ms_to_count(time);
    case TIMEUNIT_SEC:
        return sec_to_count(time);
    default:
        return 0; /* error */
    }
}

uint64_t timer_count_to_time(uint64_t count, time_unit_t unit)
{
    switch (unit) {
    case TIMEUNIT_USEC:
        return count_to_us(count);
    case TIMEUNIT_10NSEC:
        return count_to_ten_ns(count);
    case TIMEUNIT_MSEC:
        return count_to_ms(count);
    case TIMEUNIT_SEC:
        return count_to_sec(count);
    default:
        return 0; /* error */
    }
}

static inline uint64_t get_systemcounter_value(void)
{
    return read_cntpct(); /* FIXME:(igkang) Need to be rewritten using indirect call through API */
}

uint64_t timer_get_systemcounter_value(void)
{
    return get_systemcounter_value();
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

#ifdef __TEST_TIMER__
static uint64_t saved_syscnt[NR_CPUS] = {0,};
#endif

/*
 * This method handles all timer IRQ.
 */
// TODO(igkang): rename pregs and pdata for our usage.
static void timer_irq_handler(int irq, void *pregs, void *pdata)
{
    uint32_t pcpu = smp_processor_id();

#ifdef __TEST_TIMER__
    uint64_t new_syscnt = get_systemcounter_value();
    printf("time diff: %luns\n", (uint32_t)count_to_ten_ns(new_syscnt - saved_syscnt[pcpu]) * 10u);
    saved_syscnt[pcpu] = new_syscnt;
#endif

    timer_stop();

    /* TODO:(igkang) serparate timer.c into two pieces and move one into arch/arm ? */
    uint64_t now = count_to_ten_ns(get_systemcounter_value()); // * 10;

    struct timer *t;
    struct timer *tmp;
    LIST_FOR_EACH_ENTRY_SAFE(t, tmp, &active_timers[pcpu], head_active) {
        if (t->expiration < now) {
            t->callback(pregs, &t->expiration);
            /* FIXME:(igkang) would be better to use set_timer instead? */
        }
    }

    timer_maintenance();

    timer_start();
}

static void timer_requset_irq(uint32_t irq)
{
    register_irq_handler(irq, &timer_irq_handler);
    irq_hw->set_irq_type(irq, 0);  // 0 is level sensitive.
    irq_hw->enable(irq);
}

hvmm_status_t timer_hw_init(uint32_t irq) /* const struct timer_config const* timer_config)*/
{
    __ops = _timer_module.ops;

    /* FIXME:(igkang) may need per core initialization */
    if (__ops->init) {
        __ops->init();
    }

    /* TODO: (igkang) timer related call - check return value */
    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timemanager_init() /* const struct timer_config const* timer_config)*/
{
    uint32_t pcpu = smp_processor_id();

    LIST_INITHEAD(&active_timers[pcpu]);
    LIST_INITHEAD(&inactive_timers[pcpu]);

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

hvmm_status_t tm_set_timer(struct timer *t, uint64_t expiration)
{
    /* uint32_t pcpu = smp_processor_id(); */

    timer_stop();

    /* ASSERT(t != NULL); */
    tm_deactivate_timer(t);

    /* ASSERT(expiration == 0); ? */
    t->expiration = expiration;
    tm_activate_timer(t);

    /* then do maintenance routine */
    timer_maintenance();

    timer_start();
    /* FIXME:(igkang) does it really need timer_stop/start ? */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_activate_timer(struct timer *t)
{
    uint32_t pcpu = smp_processor_id();

    /* ASSERT(t != NULL); */
    LIST_DELINIT(&t->head_inactive);
    LIST_ADDTAIL(&t->head_active, &active_timers[pcpu]);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t tm_deactivate_timer(struct timer *t)
{
    uint32_t pcpu = smp_processor_id();

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
            break;
        }
    }

    /* then calculate cval from ns */
    nearest = ten_ns_to_count(nearest);

    /* then set cval */
    /* ASSERT(__ops != NULL); */
    __ops->set_cval(nearest);

    return HVMM_STATUS_SUCCESS;
}

