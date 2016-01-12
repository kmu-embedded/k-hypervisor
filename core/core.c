#include <core.h>
#include <arch/arm/rtsm-config.h>
#include <memory.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <tests.h>
#include <stdint.h>

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

hvmm_status_t khypervisor_init()
{
    printf("[%s] START\n", __func__);

    /* Initialize malloc library */
    __malloc_init();

    /* Initialize Memory */
    memory_init();

    /* Initialize Interrupt Management */
    if (interrupt_init())
        printf("[%s] interrupt initialization failed\n", __func__);

    /* Initialize Timer */
    setup_timer();
    if(timer_init(_timer_irq)) {
        printf("[%s] timer initialization failed\n", __func__);
    }

    /* Initialize Virtual Devices */
    if (vdev_init()) {
        printf("[%s] vdev initialization failed\n", __func__);
    }

    /* Begin running test code for newly implemented features */
    if (basic_tests_run(PLATFORM_BASIC_TESTS))
        printf("[%s] basic testing failed\n", __func__);

    printf("[%s] END\n", __func__);
}

