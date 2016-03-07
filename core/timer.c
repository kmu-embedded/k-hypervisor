/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */

#include <rtsm-config.h>
#include <hvmm_trace.h>
#include <arch/armv7/smp.h>
#include <core/interrupt.h>
#include <debug_print.h>
#include <core/timer.h>
#include <stdio.h>
#include <stdint.h>

static timer_callback_t __host_callback[NR_CPUS];
static timer_callback_t __guest_callback[NR_CPUS];
static uint32_t __host_tickcount[NR_CPUS];
static uint32_t __guest_tickcount[NR_CPUS];

static struct timer_ops *__ops;

/*
 * Converts time unit from/to microseconds to/from system counter count.
 */
static inline uint64_t us_to_count(uint64_t time_in_us)
{
    return time_in_us * COUNT_PER_USEC;
}

static inline uint64_t ns_to_count(uint64_t time_in_ns)
{
    return time_in_ns * (COUNT_PER_USEC / 1000);
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

static inline uint64_t count_to_ns(uint64_t count)
{
    return count / (COUNT_PER_USEC / 1000);
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
    return read_cntpct();
}

uint64_t timer_time_to_count(uint64_t time, time_unit_t unit)
{
    switch (unit) {
        case TIMEUNIT_USEC:
            return us_to_count(time);
        case TIMEUNIT_NSEC:
            return ns_to_count(time);
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
            return count_to_ns(count);
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
    if (__ops->enable)
        return __ops->enable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 *  Stops the timer.
 */
hvmm_status_t timer_stop(void)
{
    /* timer_disable() */
    if (__ops->disable)
        return __ops->disable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Sets the timer interval(microsecond).
 */
static hvmm_status_t timer_set_interval(uint32_t interval_us)
{
    /* timer_set_tval() */
    if (__ops->set_interval)
        return __ops->set_interval((uint32_t)us_to_count(interval_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

static hvmm_status_t timer_set_absolute(uint64_t absolute_us)
{
    /* timer_set_tval() */
    if (__ops->set_interval)
        return __ops->set_absolute(us_to_count(absolute_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * This method handles all timer IRQ.
 */
// TODO(igkang): rename pregs and pdata for our usage.
static void timer_handler(int irq, void *pregs, void *pdata)
{
    uint32_t pcpu = smp_processor_id();

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
    gic_configure_irq(irq, 0);  // 0 is level sensitive.
    gic_enable_irq(irq);
}

static hvmm_status_t timer__host_set_callback(timer_callback_t func, uint32_t interval_us)
{
    uint32_t pcpu = smp_processor_id();

    __host_callback[pcpu] = func;
    __host_tickcount[pcpu] = interval_us / TICK_PERIOD_US;
    /* FIXME:(igkang) hardcoded */

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t timer__guest_set_callback(timer_callback_t func, uint32_t interval_us)
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
        timer__host_set_callback(timer->callback, timer->interval);
        timer_set_interval(TICK_PERIOD_US);
        timer_start();
    } else
        timer__guest_set_callback(timer->callback, timer->interval);

    /* TODO:(igkang) add code to handle guest_callback count (for vdev)  */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(uint32_t irq)
{
    __ops = _timer_module.ops;

    if (__ops->init)
        __ops->init();

    /* TODO: (igkang) timer related call - check return value */
    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}
