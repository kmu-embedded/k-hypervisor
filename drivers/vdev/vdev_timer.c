#include <stdio.h>
#include <vdev.h>
#include <core/timer.h>
#include <arch/irq.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <irq-chip.h>
#include <drivers/vdev/vdev_timer.h>

// #define DEBUG

#define VTIMER_IRQ 30 /* NS PL1 Timer */

extern struct virq_chip *virq_hw;

/*
   CP64(CNTPCT)
   CP64(CNTVCT)
   CP64(CNTP_CVAL)
   CP64(CNTV_CVAL)

   CP32(CNTFRQ)
   CP32(CNTKCTL)
   CP32(CNTP_TVAL)
   CP32(CNTP_CTL)
   CP32(CNTV_TVAL)
   CP32(CNTV_CTL)
   */

int vdev_timer_access32(uint8_t read, uint32_t what, uint32_t *rt)
{
    struct vcpu *vcpu = get_current_vcpu();
    struct vdev_timer *v = &vcpu->vtimer;
    uint32_t *target = NULL;
    uint32_t tmp = 0;

    switch (what) {
        case CP32(CNTFRQ)    :
            target = &v->frq;
            break;
        case CP32(CNTKCTL)   :
            target = &v->k_ctl;
            break;
        case CP32(CNTP_CTL)  :
            target = &v->p_ctl;
            break;
        case CP32(CNTV_CTL)  :
            target = &v->v_ctl;
            break;

        case CP32(CNTP_TVAL) :
            if (read) {
                target = &tmp;
                tmp = (uint32_t)
                    ( v->p_cval - (timer_get_syscounter() - v->p_ct_offset) );
            } else {
                /* FIXME:(igkang) sign extension of *rt needed */
                v->p_cval = (timer_get_syscounter() - v->p_ct_offset + *rt);
                return 0;
            }
        case CP32(CNTV_TVAL) :
            if (read) {
                tmp = (uint32_t)
                    ( v->v_cval - (timer_get_syscounter() - v->p_ct_offset) );
            } else {
                /* FIXME:(igkang) sign extension of *rt needed */
                v->v_cval = (timer_get_syscounter() - v->p_ct_offset + *rt);
                return 0;
            }
            /* do subtraction and set cval */
            break;
        default:
            return 1;
            break;
    }

    if (read) {
        *rt = *target;
    } else { /* write */
        *target = *rt;
    }

    return 0;
}

int vdev_timer_access64(uint8_t read, uint32_t what, uint32_t *rt_low, uint32_t *rt_high)
{
    struct vcpu *vcpu = get_current_vcpu();
    struct vdev_timer *v = &vcpu->vtimer;
    uint64_t *target = NULL;
    uint64_t tmp = 0;

    switch (what) {
        case CP64(CNTPCT)    :
        case CP64(CNTVCT)    :
            if (!read) {
                return 1;
            } else {
                target = &tmp;
                tmp = timer_get_syscounter() + v->p_ct_offset;
            }
            /* do something */
            break;
        case CP64(CNTP_CVAL) :
            target = &v->p_cval;
            break;
        case CP64(CNTV_CVAL) :
            target = &v->v_cval;
            break;
        default:
            return 1;
            break;
    }

    if (read) {
        *rt_low  = (uint32_t) (*target & 0xFFFFFFFF);
        *rt_high = (uint32_t) (*target >> 32);
    } else { /* write */
        *target = ((uint64_t) *rt_high << 32) | (uint64_t) *rt_low;
    }

    return 0;
}

