#include <stdio.h>
#include <vdev.h>
#include <core/timer.h>
#include <arch/irq.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <irq-chip.h>

// #define DEBUG

#define VTIMER_IRQ 30 /* NS PL1 Timer */

extern struct virq_chip *virq_hw;

/* represents a generic timer w/o SE,VE (NS PL1, PL0 timer) */
struct vdev_timer {
    /* data for hypervisor */
    uint64_t p_ct_offset;

    /* regs for guest */
    uint32_t k_ctl;

    uint32_t p_ctl;
    uint64_t p_cval;

    uint32_t v_ctl;
    uint64_t v_cval;
    uint64_t v_off;
};

/*
CP64  CNTPCT
CP64  CNTVCT
CP64  CNTP_CVAL
CP64  CNTV_CVAL
CP64  CNTVOFF
CP64  CNTHP_CVAL

CP32  CNTFRQ
CP32  CNTKCTL
CP32  CNTP_TVAL
CP32  CNTP_CTL
CP32  CNTV_TVAL
CP32  CNTV_CTL
CP32  CNTHCTL
CP32  CNTHP_TVAL
CP32  CNTHP_CTL
 */

bool vdev_timer_access32(uint8_t dir, uint32_t *rt) {
    return false;
}

bool vdev_timer_access64(uint8_t dir, uint32_t *rt_low, uint32_t *rt_high) {
    return false;
}

