#ifndef __VDEV_TIMER_H__
#define __VDEV_TIMER_H__

#include <core/timer.h>

#define VTIMER_PERIODIC

/* represents a generic timer w/o SE,VE (Physical, virtual timer) */
struct vdev_timer {
    /* data for hypervisor */
    uint64_t p_ct_offset;

    /* regs for guest */
    uint32_t frq;
    uint32_t k_ctl;

    uint32_t p_ctl;
    uint64_t p_cval;
    /* tval is a diff between CNTPCT and CNTP_CVAL */

    uint32_t v_ctl;
    uint64_t v_cval;
    // uint64_t v_off;
    /* ARM DDI 0406C.c B8-1969 claims that CNTVOFF is only for VE */

#ifdef VTIMER_PERIODIC
    bool periodic_mode;
    uint32_t periodic_interval;
#endif

    struct timer swtimer; /* for Physical Timer (?) */
};

int vdev_timer_access64(uint8_t read, uint32_t what, uint32_t *rt_low, uint32_t *rt_high);
int vdev_timer_access32(uint8_t read, uint32_t what, uint32_t *rt);
void init_vdev_timer(struct vdev_timer *v);

#endif /* __VDEV_TIMER_H__ */
