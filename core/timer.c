/*
 * timer.c
 * --------------------------------------
 * Implementation of ARMv7 Generic Timer
 * Responsible: Inkyu Han
 */

#include <stdio.h>
#include <debug_print.h>
#include <stdint.h>
#include <rtsm-config.h>

#include <hvmm_trace.h>

#include <core/timer.h>
#include <core/interrupt.h>
#include <arch/armv7/smp.h>

static timer_callback_t _host_callback[NUM_CPUS];
static timer_callback_t _guest_callback[NUM_CPUS];
static uint32_t _host_tickcount[NUM_CPUS];
static uint32_t _guest_tickcount[NUM_CPUS];

static struct timer_ops *_ops;

/*
 * Converts time unit from/to microseconds to/from system counter count.
 */
static inline uint64_t _usec2count(uint64_t time_us)
{
    return time_us * COUNT_PER_USEC;
}
static inline uint64_t _count2usec(uint64_t count)
{
    return count / COUNT_PER_USEC;
}
static inline uint64_t _get_syscnt(void)
{
    return read_cntpct();
}

uint64_t timer_usec2count(uint64_t time_us)
{
    return _usec2count(time_us);
}

uint64_t timer_count2usec(uint64_t count)
{
    return _count2usec(count);
}

uint64_t timer_get_syscnt(void)
{
    return _get_syscnt();
}

/*
 * Starts the timer.
 */
hvmm_status_t timer_start(void)
{
    /* timer_enable() */
    if (_ops->enable)
        return _ops->enable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 *  Stops the timer.
 */
hvmm_status_t timer_stop(void)
{
    /* timer_disable() */
    if (_ops->disable)
        return _ops->disable();

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * Sets the timer interval(microsecond).
 */
static hvmm_status_t timer_set_interval(uint32_t interval_us)
{
    /* timer_set_tval() */
    if (_ops->set_interval)
        return _ops->set_interval((uint32_t)_usec2count(interval_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

static hvmm_status_t timer_set_absolute(uint64_t absolute_us)
{
    /* timer_set_tval() */
    if (_ops->set_interval)
        return _ops->set_absolute(_usec2count(absolute_us));

    return HVMM_STATUS_UNSUPPORTED_FEATURE;
}

/*
 * This method handles all timer IRQ.
 */
static void timer_handler(int irq, void *pregs, void *pdata)
{
    uint32_t cpu = smp_processor_id();

    timer_stop();
    if (_host_callback[cpu] && --_host_tickcount[cpu] == 0) {
        _host_callback[cpu](pregs, &_host_tickcount[cpu]);
        _host_tickcount[cpu] /= TICK_PERIOD_US;
    }
    if (_guest_callback[cpu] && --_guest_tickcount[cpu] == 0) {
        _guest_callback[cpu](pregs, &_guest_tickcount[cpu]);
        _guest_tickcount[cpu] /= TICK_PERIOD_US;
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

static hvmm_status_t timer_host_set_callback(timer_callback_t func, uint32_t interval_us)
{
    uint32_t cpu = smp_processor_id();

    _host_callback[cpu] = func;
    _host_tickcount[cpu] = interval_us / TICK_PERIOD_US;
    /* FIXME:(igkang) hardcoded */

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t timer_guest_set_callback(timer_callback_t func, uint32_t interval_us)
{
    uint32_t cpu = smp_processor_id();

    _guest_callback[cpu] = func;
    _guest_tickcount[cpu] = interval_us / TICK_PERIOD_US;
    /* FIXME:(igkang) hardcoded */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_set(struct timer_val *timer, uint32_t host)
{
    if (host) {
        timer_stop();
        timer_host_set_callback(timer->callback, timer->interval_us);
        timer_set_interval(TICK_PERIOD_US);
        timer_start();
    } else
        timer_guest_set_callback(timer->callback, timer->interval_us);

    /* TODO:(igkang) add code to handle guest_callback count (for vdev)  */

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t timer_init(uint32_t irq)
{
    _ops = _timer_module.ops;

    if (_ops->init)
        _ops->init();

    /* TODO: (igkang) timer related call - check return value */
    timer_requset_irq(irq);

    return HVMM_STATUS_SUCCESS;
}
