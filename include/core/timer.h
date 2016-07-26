#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include "../types.h"

#include <lib/list.h>

#define GUEST_TIMER 0
#define HOST_TIMER 1

/* bigger time units into nanoseconds */
#define NOW()     ((uint64_t)timer_get_timenow())
#define SEC(s)    ((uint64_t)((s)  * 1000000000ULL))
#define MSEC(ms)  ((uint64_t)((ms) * 1000000ULL))
#define USEC(us)  ((uint64_t)((us) * 1000ULL))

typedef void(*timer_callback_t)(void *pdata, uint64_t *expiration);

struct timer {
    struct list_head head_active;
    struct list_head head_inactive;

    uint32_t state;
    uint64_t expiration;
    timer_callback_t callback;
};

struct timer_ops {
    hvmm_status_t (*init)(void);
    hvmm_status_t (*enable)(void);
    hvmm_status_t (*disable)(void);
    uint64_t (*get_counter)(void);
    hvmm_status_t (*set_absolute)(uint64_t);
    hvmm_status_t (*set_interval_relative)(uint32_t);
    hvmm_status_t (*set_interval_absolute)(uint64_t);
    hvmm_status_t (*dump)(void);
};

struct timer_module {
    uint32_t version;
    const char *id;
    const char *name;
    const char *author;
    struct timer_ops *ops;
};

extern struct timer_module _timer_module;
/*
 * Calling this function is required only once in the entire system
 * prior to calls to other functions of Timer module.
 */
hvmm_status_t timer_hw_init(uint32_t irq);

hvmm_status_t timemanager_init();
hvmm_status_t tm_register_timer(struct timer *t, timer_callback_t callback);
hvmm_status_t tm_set_timer(struct timer *t, uint64_t expiration, bool timer_stopstart);
hvmm_status_t tm_activate_timer(struct timer *t);
hvmm_status_t tm_deactivate_timer(struct timer *t);

hvmm_status_t timer_stop(void);
hvmm_status_t timer_start(void);

uint64_t timer_count_to_time_ns(uint64_t count);
uint64_t timer_time_to_count_ns(uint64_t time);
uint64_t timer_get_syscounter(void);
uint64_t timer_get_timenow(void);

#endif
