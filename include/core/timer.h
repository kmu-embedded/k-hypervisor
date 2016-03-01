#ifndef __TIMER_H__
#define __TIMER_H__

#include "hvmm_types.h"
#include "stdint.h"


#define GUEST_TIMER 0
#define HOST_TIMER 1

typedef void(*timer_callback_t)(void *pdata, uint32_t *delay_tick);

struct timer_val {
    uint32_t interval_us;
    timer_callback_t callback;
};

struct timer_ops {
    hvmm_status_t (*init)(void);
    hvmm_status_t (*enable)(void);
    hvmm_status_t (*disable)(void);
    hvmm_status_t (*set_interval)(uint64_t);
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
hvmm_status_t timer_init(uint32_t irq);
hvmm_status_t timer_set(struct timer_val *timer, uint32_t host);
hvmm_status_t timer_stop(void);
hvmm_status_t timer_start(void);

uint64_t timer_usec2count(uint64_t time_us);
uint64_t timer_count2usec(uint64_t count);
uint64_t timer_get_syscnt(void);
#endif
