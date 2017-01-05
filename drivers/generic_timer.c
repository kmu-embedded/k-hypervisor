#include <stdio.h>
#include <arch/armv7.h>
#include <arch/irq.h>

#include <core/timer.h>

#include <config.h>

enum generic_timer_type {
    GENERIC_TIMER_HYP,      /* IRQ 26 PL2 Hyp */
    GENERIC_TIMER_VIR,      /* IRQ 27 Virtual */
    GENERIC_TIMER_NSP,      /* IRQ 30 Non-secure Physical */
    GENERIC_TIMER_NUM_TYPES
};

/** @brief Configures time interval by PL2 physical timerValue register.
 *  Read CNTHP_TVAL into R0.
 *
 *  "CNTHP_TVAL" characteristics
 *  -Holds the timer values for the Hyp mode physical timer.
 *  -Only accessible from Hyp mode, or from
 *   Monitor mode when SCR.NS is  set to 1.
 */
static hvmm_status_t generic_timer_set_tval(enum generic_timer_type timer_type,
                                            uint32_t tval)
{
    hvmm_status_t result = HVMM_STATUS_SUCCESS;

    switch (timer_type) {
    case GENERIC_TIMER_HYP:
        write_cp32(tval, CNTHP_TVAL);
        break;
    case GENERIC_TIMER_VIR:
        write_cp32(tval, CNTV_TVAL);
        break;
    case GENERIC_TIMER_NSP:
        write_cp32(tval, CNTP_TVAL);
        break;
    default:
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }

    return result;
}

static hvmm_status_t generic_timer_set_cval(enum generic_timer_type timer_type,
                                            uint64_t cval)
{
    hvmm_status_t result = HVMM_STATUS_SUCCESS;

    switch (timer_type) {
    case GENERIC_TIMER_HYP:
        write_cp64(cval, CNTHP_CVAL);
        break;
    case GENERIC_TIMER_VIR:
        write_cp64(cval, CNTV_CVAL);
        break;
    case GENERIC_TIMER_NSP:
        write_cp64(cval, CNTP_CVAL);
        break;
    default:
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }

    return result;
}

static hvmm_status_t generic_timer_enable(enum generic_timer_type timer_type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;

    switch (timer_type) {
    case GENERIC_TIMER_HYP:
        ctrl = read_cp32(CNTHP_CTL);
        ctrl |= ARM_CNTCTL_ENABLE;
        ctrl &= ~ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTHP_CTL);
        break;
    case GENERIC_TIMER_VIR:
        ctrl = read_cp32(CNTV_CTL);
        ctrl |= ARM_CNTCTL_ENABLE;
        ctrl &= ~ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTV_CTL);
        break;
    case GENERIC_TIMER_NSP:
        ctrl = read_cp32(CNTP_CTL);
        ctrl |= ARM_CNTCTL_ENABLE;
        ctrl &= ~ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTP_CTL);
        break;
    default:
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }

    return result;
}

static hvmm_status_t generic_timer_disable(enum generic_timer_type timer_type)
{
    uint32_t ctrl;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;

    switch (timer_type) {
    case GENERIC_TIMER_HYP:
        ctrl = read_cp32(CNTHP_CTL);
        ctrl &= ~ARM_CNTCTL_ENABLE;
        ctrl |= ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTHP_CTL);
        break;
    case GENERIC_TIMER_VIR:
        ctrl = read_cp32(CNTV_CTL);
        ctrl &= ~ARM_CNTCTL_ENABLE;
        ctrl |= ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTV_CTL);
        break;
    case GENERIC_TIMER_NSP:
        ctrl = read_cp32(CNTP_CTL);
        ctrl &= ~ARM_CNTCTL_ENABLE;
        ctrl |= ARM_CNTCTL_IMASK;
        write_cp32(ctrl, CNTP_CTL);
        break;
    default:
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }

    return result;
}

static hvmm_status_t timer_disable()
{
    return generic_timer_disable(GENERIC_TIMER_HYP);
}

static hvmm_status_t timer_enable()
{
    return generic_timer_enable(GENERIC_TIMER_HYP);
}

static hvmm_status_t timer_set_cval(uint64_t val)
{
    return generic_timer_set_cval(GENERIC_TIMER_HYP, val);
}

uint32_t have_set_cval_by_delta_ever_called[NR_CPUS] = {0};
static hvmm_status_t timer_set_cval_by_delta(uint64_t val)
{
    uint64_t previous_cval; /* FIXME:(igkang) this calculation have to be moved to timer.c */

    if (have_set_cval_by_delta_ever_called[NR_CPUS]) {
        previous_cval = read_cp64(CNTHP_CVAL);
        val = previous_cval + val;
    } else { /* when if this is the very first call of timer_set_cval_by_delta */
        previous_cval = read_cp64(CNTPCT);
        val = previous_cval + val;
    }

    return generic_timer_set_cval(GENERIC_TIMER_HYP, val);
}

static hvmm_status_t timer_set_tval(uint32_t tval)
{
    return generic_timer_set_tval(GENERIC_TIMER_HYP, tval);
}

/** @brief dump at time.
 *  @todo have to write dump with meaningful printing.
 */
static hvmm_status_t timer_dump(void)
{
    return HVMM_STATUS_SUCCESS;
}

static uint64_t timer_read_phys_counter(void)
{
    return read_cp64(CNTPCT);
}

#if 0
static uint64_t timer_read_virt_counter(void)
{
    return read_cp64(CNTVCT);
}
#endif

#if 0
static hvmm_status_t timer_init(void)
{
// #define CONFIG_TIMER_ACCESS_TRAP
#ifdef CONFIG_TIMER_ACCESS_TRAP
    uint32_t val;

    val = read_cp32(CNTHCTL);
    write_cp32(val & ~(ARM_CNTHCTL_PL1PCEN | ARM_CNTHCTL_PL1PCTEN), CNTHCTL);
#endif

    return HVMM_STATUS_SUCCESS;
}
#endif

/* TODO:(igkang) add/rename functions - by phys/virt, Hyp/Normal */
struct timer_ops _timer_ops = {
//    .init = timer_init,
    .enable = timer_enable,
    .disable = timer_disable,
    .get_counter = timer_read_phys_counter,
    .set_absolute = timer_set_cval,
    .set_interval_relative = timer_set_tval,
    .set_interval_absolute = timer_set_cval_by_delta,
    .dump = timer_dump,
};

struct timer_module _timer_module = {
    .name = "K-Hypervisor Timer Module",
    .author = "Kookmin Univ.",
    .ops = &_timer_ops,
};
