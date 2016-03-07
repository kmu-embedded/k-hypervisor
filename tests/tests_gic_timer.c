#include <stdio.h>
#include "../arch/arm/gic-v2.h"
#include "../arch/arm/vgic.h"
#include <core/vm/vcpu.h>
#include <hvmm_trace.h>
#include <arch/armv7.h>
#include <core/timer.h>
#include <core/scheduler.h>
#include <rtsm-config.h>
#include <core/interrupt.h>

static void test_start_timer(void)
{
    uint32_t ctl;
    uint32_t tval;
    uint64_t pct;
    HVMM_TRACE_ENTER();
    /* every second */
    tval = read_cntfrq();
    write_cntp_tval(tval);
    pct = read_cntpct();
    printf("cntpct %llu\n", pct);
    printf("cntp_tval %llu\n", tval);
    /* enable timer */
    ctl = read_cntp_ctl();
    ctl |= 0x1;
    write_cntp_ctl(ctl);
    HVMM_TRACE_EXIT();
}

void interrupt_nsptimer(int irq, void *pregs, void *pdata)
{
    uint32_t ctl;
    struct core_regs *regs = pregs;
    printf("=======================================\n\r");
    HVMM_TRACE_ENTER();
    /* Disable NS Physical Timer Interrupt */
    ctl = read_cntp_ctl();
    ctl &= ~(0x1);
    write_cntp_ctl(ctl);
    /* Trigger another interrupt */
    test_start_timer();
    /* Test guest context switch */
    if ((regs->cpsr & 0x1F) != 0x1A) {
        /* Not from Hyp, switch the guest context */
//        vcpu_dump_regs(regs);
//        vcpu_regs_dump(GUEST_VERBOSE_ALL, regs);
//        print_core_regs(regs);
        /* FIXME:(igkang) sched */ // guest_switchto(sched_policy_determ_next());
    }
    HVMM_TRACE_EXIT();
    printf("=======================================\n\r");
}
#if defined(CFG_BOARD_ARNDALE)
void interrupt_pwmtimer(void *pdata)
{
    pwm_timer_disable_int();
    printf("=======================================\n\r");
    HVMM_TRACE_ENTER();
    HVMM_TRACE_EXIT();
    printf("=======================================\n\r");
    pwm_timer_enable_int();
}

hvmm_status_t hvmm_tests_gic_pwm_timer(void)
{
    /* Testing pwm timer event (timer1, Interrupt ID : 69),
     * Cortex-A15 exynos5250
     * - Periodically triggers timer interrupt
     * - Just print printf
     */
    HVMM_TRACE_ENTER();
    pwm_timer_init();
    pwm_timer_set_callback(&interrupt_pwmtimer);
    pwm_timer_enable_int();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
#endif

hvmm_status_t hvmm_tests_gic_timer(void)
{
    /* Testing Non-secure Physical Timer Event
     * (PPI2, Interrupt ID:30), Cortex-A15
     * - Periodically triggers timer interrupt
     * - switches guest context at every timer interrupt
     */
    HVMM_TRACE_ENTER();
    /* handler */
    register_irq_handler(30, &interrupt_nsptimer);
    /* configure and enable interrupt */
    gic_configure_irq(30, IRQ_LEVEL_TRIGGERED);
    gic_enable_irq(30);
    /* start timer */
    test_start_timer();
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

void callback_test_timer(void *pdata, uint32_t *delay_tick)
{
    vmid_t vmid;
    HVMM_TRACE_ENTER();
    vmid = get_current_vcpuid();
    printf("Injecting IRQ 30 to Guest:%d\n", vmid);

    /* SW VIRQ, No PIRQ */
    virq_inject(vmid, 30, 0, INJECT_SW);

    /* FIXME:(igkang) hardcoded */
    *delay_tick = 1 * TICKTIME_1MS / 50;

    HVMM_TRACE_EXIT();
}

hvmm_status_t hvmm_tests_vgic(void)
{
    struct timer timer;
    /* VGIC test
     *  - Implementation Not Complete
     *  - TODO: specify guest to receive the virtual IRQ
     *  - Once the guest responds to the IRQ, Virtual Maintenance
     *    Interrupt service routine should be called
     *      -> ISR implementation is empty for the moment
     *      -> This should handle completion of deactivation and further
     *         injection if there is any pending virtual IRQ
     */
    timer.interval_us = TICKTIME_1MS;
    timer.callback = &callback_test_timer;
    timer_set(&timer, HOST_TIMER);

    return HVMM_STATUS_SUCCESS;
}
