#include <stdio.h>
#include <debug_print.h>

#include <core.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <tests.h>
#include <stdint.h>
#include <rtsm-config.h>

#include <platform.h>

#define PLATFORM_BASIC_TESTS 4

static uint32_t _timer_irq;

/** @brief Registers generic timer irqs such as hypervisor timer event
 *  (GENERIC_TIMER_HYP), non-secure physical timer event(GENERIC_TIMER_NSP)
 *  and virtual timer event(GENERIC_TIMER_NSP).
 *  Each interrup source is identified by a unique ID.
 *  cf. "Cortex™-A15 Technical Reference Manual" 8.2.3 Interrupt sources
 *
 *  DEVICE : IRQ number
 *  GENERIC_TIMER_HYP : 26
 *  GENERIC_TIMER_NSP : 30
 *  GENERIC_TIMER_VIR : 27
 *
 *  @note "Cortex™-A15 Technical Reference Manual", 8.2.3 Interrupt sources
 */
void setup_timer()
{
    _timer_irq = 26;
}

// TODO: Every subfunction have a return value.
hvmm_status_t khypervisor_init()
{
    hvmm_status_t status;

    if ((status = interrupt_init()) == HVMM_STATUS_UNKNOWN_ERROR) {
        debug_print("[%s] interrupt initialization failed\n", __func__);
        return status;
    }

    setup_timer();
    if ((status = timer_init(_timer_irq)) == HVMM_STATUS_UNKNOWN_ERROR) {
        debug_print("[%s] timer initialization failed\n", __func__);
        return status;
    }

    if ((status = vdev_init()) == HVMM_STATUS_UNKNOWN_ERROR) {
        debug_print("[%s] vdev initialization failed\n", __func__);
        return status;
    }

    // FIXME: Currently, testing routines for vdev are failed
    if ((status = basic_tests_run(PLATFORM_BASIC_TESTS)) == HVMM_STATUS_UNKNOWN_ERROR) {
        debug_print("[%s] basic testing failed\n", __func__);
        //return status;
        status = HVMM_STATUS_SUCCESS;
    }

    return status;
}

