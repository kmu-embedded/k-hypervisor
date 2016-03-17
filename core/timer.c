/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */

#include <config.h>
#include <arch/armv7.h>
#include <arch/irq.h>
#include <debug.h>
#include <core/timer.h>
#include <stdio.h>
#include <stdint.h>
#include <irq-chip.h>

static timer_callback_t __host_callback[NR_CPUS];
static timer_callback_t __guest_callback[NR_CPUS];
static uint32_t __host_tickcount[NR_CPUS];
static uint32_t __guest_tickcount[NR_CPUS];
static struct timer_ops *__ops;

/* TODO:(igkang) Conditional timeunit function definition based on CFG_CNTFRQ */
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

static inline uint64_t get_systemcounter_value(void)
{
    return read_cntpct(); /* FIXME:(igkang) Need to be rewritten using indirect call through API */
}

uint64_t timer_time_to_count(uint64_t time, time_unit_t unit)
{
    switch (unit) {
    case TIMEUNIT_USEC:
        return us_to_count(time);
    case TIMEUNIT_NSEC:
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
    case TIMEUNIT_NSEC:
        return count_to_ten_ns(count);
    case TIMEUNIT_MSEC:
        return count_to_ms(count);
    case TIMEUNIT_SEC:
        return count_to_sec(count);
    default:
        return 0; /* error */
    }
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

/* FIXME:(igkang) commented out to avoid -Wall -Werror */
#if 0
/*
 * Sets the timer interval(microsecond).
 */
static hvmm_status_t timer_set_interval(uint32_t interval_us)
{
    /* timer_set_tval() */
    if (__ops->set_interval) {
        return __ops->set_interval((uint32_t)us_to_count(interval_us));
    }

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}
#endif

static hvmm_status_t timer_set_absolute(uint64_t absolute_us)
{
    /* timer_set_tval() */
    if (__ops->set_interval) {
        return __ops->set_absolute(us_to_count(absolute_us));
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
static void timer_handler(int irq, void *pregs, void *pdata)
{
    uint32_t pcpu = smp_processor_id();

#ifdef __TEST_TIMER__
    uint64_t new_syscnt = get_systemcounter_value();
    printf("time diff: %luns\n", (uint32_t)count_to_ten_ns(new_syscnt - saved_syscnt[pcpu]) * 10u);
    saved_syscnt[pcpu] = new_syscnt;
#endif

    timer_stop();

    // TODO(igkang): remove __host_tickcount.
    if (__host_callback[pcpu] && --__host_tickcount[pcpu] == 0) {
        __host_callback[pcpu](pregs, &__host_tickcount[pcpu]);
        __host_tickcount[pcpu] /= TICK_PERIOD_US;
    }
    if (__guest_callback[pcpu] && --__guest_tickcount[pcpu] == 0) {
        __guest_callback[pcpu](pregs, &__guest_tickcount[pcpu]);
        __guest_tickcount[pcpu] /= TICK_PERIOD_US;
    }

    timer_set_absolute(TICK_PERIOD_US);
    timer_start();
}

static void timer_requset_irq(uint32_t irq)
{
    register_irq_handler(irq, &timer_handler);
    irq_hw->set_irq_type(irq, 0);  // 0 is level sensitive.
    irq_hw->enable(irq);
}

static hvmm_status_t timer_host_set_callback(timer_callback_t func, uint32_t interval_us)
{
    uint32_t pcpu = smp_processor_id();

    __host_callback[pcpu] = func;
    __host_tickcount[pcpu] = interval_us / TICK_PERIOD_US;
    /* FIXME:(igkang) hardcoded */

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t timer_guest_set_callback(timer_callback_t func, uint32_t interval_us)
{
    uint32_t pcpu = smp_processor_id();

    __guest_callback[pcpu] = func;
    __guest_tickcount[pcpu] = interval_us / TICK_PERIOD_US;
    /* FIXME:(igkang) hardcoded */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set(struct timer *timer, uint32_t host)
{
    if (host) {
        timer_stop();
        timer_host_set_callback(timer->callback, timer->interval);
        timer_set_absolute(TICK_PERIOD_US);
        timer_start();
    } else {
        timer_guest_set_callback(timer->callback, timer->interval);
    }

    /* TODO:(igkang) add code to handle guest_callback count (for vdev)  */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(uint32_t irq)
{
    __ops = _timer_module.ops;

    if (__ops->init) {
        __ops->init();
    }

    /* TODO: (igkang) timer related call - check return value */
    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}
