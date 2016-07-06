#include <stdio.h>
#include <vdev.h>
#include <core/timer.h>
#include <arch/irq.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <irq-chip.h>
#include <drivers/vdev/vdev_timer.h>

// #include <lib/list.h>

// #define DEBUG

/*
 * CHECK LIST
 * [v] Generic Timer PL1/PL0 access setting (to cause exception)
 * [v] vDev Timer instance init
 * [ ] Timer access handler
 *      [v] Skeleton
 *      [v] Set SW timer value (tm_set_timer)
 *      [ ] Handle Enable/Disable bits, IMASK --> enable/disable swtimer?
 *           [ ] Timer enable bit, Interrupt Mask
 *           [ ] Generic Timer access control bit
 * [ ] Timer IRQ handler/injector
 *      [v] Skeleton
 *      [v] Set ISTATUS
 *      [v] Check ENABLE/IMASK
 *      [ ] Check CPSR.I (IRQ mask bit)
 * [ ] Support both of CNTP/CNTV
 *
 * [ ] ??per-vtimer cntfrq
 *
 * [ ] Periodic mode support
 */

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

inline static int vdev_timer_set_swtimer(struct vdev_timer *v)
{
    // hvmm_status_t tm_set_timer(struct timer *t, uint64_t expiration, bool timer_stopstart)
    // uint64_t timer_count_to_time_ns(uint64_t count)
    // uint64_t timer_time_to_count_ns(uint64_t time)

    uint64_t exp = timer_count_to_time_ns(v->p_cval + v->p_ct_offset);
    tm_set_timer(&v->swtimer, exp, true);

    return 0;
}

int vdev_timer_access32(uint8_t read, uint32_t what, uint32_t *rt)
{
    struct vcpu *vcpu = get_current_vcpu();
    struct vdev_timer *v = &vcpu->vtimer;
    uint32_t *target = NULL;
    uint32_t tmp = 0;

    // printf("<trace> %s %d %d\n", __func__, __LINE__, vcpu->pcpuid);

    switch (what) {
        case CP32(CNTFRQ)    :
            target = &v->frq;
            break;
        case CP32(CNTKCTL)   :
            target = &v->k_ctl;
            break;
        /* TODO:(igkang) check permission before reading */
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
                vdev_timer_set_swtimer(v);
                return 0;
            }
            break;
        case CP32(CNTV_TVAL) :
            if (read) {
                target = &tmp;
                tmp = (uint32_t)
                    ( v->v_cval - (timer_get_syscounter() - v->p_ct_offset) );
            } else {
                /* FIXME:(igkang) sign extension of *rt needed */
                v->v_cval = (timer_get_syscounter() - v->p_ct_offset + *rt);
                // vdev_timer_set_swtimer(v); /* now NSP timer only */
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
    bool cval_changed = false;

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
            cval_changed = true;
            break;
        case CP64(CNTV_CVAL) :
            target = &v->v_cval;
            // cval_changed = true; /* now NSP timer only */
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

    if (cval_changed) {
        vdev_timer_set_swtimer(v);
    }

    return 0;
}

/* FIXME:(igkang) The macro "container_of()" in lib/list.h is not complete.
 * References:
 *      http://fxr.watson.org/fxr/source/contrib/vchiq/interface/compat/list.h
 */
#define container_of2(ptr, type, member)                         \
({                                                              \
        __typeof(((type *)0)->member) *_p = (ptr);              \
        (type *)((char *)_p - offsetof(type, member));          \
})

#define PRINTNOW() printf("%d %s %d: %x%x\n", pcpu, __func__, __LINE__, read_cp64(CNTPCT))
/* TODO:(igkang) need to rewrite core/timer code to store pdata in each "struct timer" instance */
void vdev_timer_handler(void *pdata, uint64_t *expiration) {
    *expiration = 0;

    uint32_t pcpu = smp_processor_id();
    // uint64_t t1, t2;

    PRINTNOW();
    /* do IRQ injection to vCPU of vdev_timer instance which we currently handling */
    struct timer *t = container_of2(expiration, struct timer, expiration);
    struct vdev_timer *v = container_of2(t, struct vdev_timer, swtimer);
    struct vcpu *vcpu = container_of2(v, struct vcpu, vtimer);
    PRINTNOW();

    /* check ENABLE */
    if ((v->p_ctl & 1u) == 1) {
        v->p_ctl |= 1u << 2; /* set ISTATUS */

        /* check IMASK */
        if ((v->p_ctl & (1u << 1)) == 0u) {
            PRINTNOW();
            /* TODO:(igkang) may need CPSR IRQ bit check */
            virq_hw->forward_irq(vcpu, VTIMER_IRQ, VTIMER_IRQ, INJECT_SW);
            PRINTNOW();
        }
    }

    /* FIXME:(igkang) remember, we support only NSP timer for now */
}

void init_vdev_timer(struct vdev_timer *v)
{
    /* Reset _CTLs
     *
     * for reset values, see ARMv7 reference manual
     *
     * CHECK LIST
     * [v] CNTFRQ
     * [v] CNTKCTL
     * [v] CNTP_CTL
     * [v] CNTV_CTL
     */

    v->frq = 100000000u; /* 100MHz */
    v->k_ctl = 0u; /* PL0{P,V}TEN, EVENTI, EVNTDIR, EVNTEN, PL0{P,V}CTEN = 0,0, 0000, 0, 0, 0,0 */
    v->p_ctl = 0u | 1u << 1; /* ISTATUS, IMASK, ENABLE = 0, 1, 0 */
    v->v_ctl = 0u | 1u << 1; /* TODO:(igkang) Define bit numbers */

    // set couter offset
    v->p_ct_offset = timer_get_syscounter();

    // init and register sw timer
    tm_register_timer(&v->swtimer, vdev_timer_handler);
}

