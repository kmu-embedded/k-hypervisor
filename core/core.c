#include <core.h>
#include <arch/arm/rtsm-config.h>
#include <interrupt.h>
#include <timer.h>
#include <vdev.h>
#include <tests.h>
#include <stdint.h>

#define PLATFORM_BASIC_TESTS 4

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                        \
        name[id].map[_pirq].virq = _virq;       \
        name[id].map[_virq].pirq = _pirq;       \
    } while (0)

static uint32_t _timer_irq;
static struct guest_virqmap _guest_virqmap[NUM_GUESTS_STATIC];

/*
 * Creates a mapping table between PIRQ and VIRQ.vmid/pirq/coreid.
 * Mapping of between pirq and virq is hard-coded.
 */
void setup_interrupt()
{
    int i, j;
    struct virqmap_entry *map;

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        map = _guest_virqmap[i].map;
        for (j = 0; j < MAX_IRQS; j++) {
            map[j].enabled = GUEST_IRQ_DISABLE;
            map[j].virq = VIRQ_INVALID;
            map[j].pirq = PIRQ_INVALID;
        }
    }

    /*
     *  vimm-0, pirq-69, virq-69 = pwm timer driver
     *  vimm-0, pirq-32, virq-32 = WDT: shared driver
     *  vimm-0, pirq-34, virq-34 = SP804: shared driver
     *  vimm-0, pirq-35, virq-35 = SP804: shared driver
     *  vimm-0, pirq-36, virq-36 = RTC: shared driver
     *  vimm-0, pirq-38, virq-37 = UART: dedicated driver IRQ 37 for guest 0
     *  vimm-1, pirq-39, virq-37 = UART: dedicated driver IRQ 37 for guest 1
     *  vimm-2, pirq,40, virq-37 = UART: dedicated driver IRQ 37 for guest 2
     *  vimm-3, pirq,48, virq-37 = UART: dedicated driver IRQ 38 for guest 3 -ch
     *  vimm-0, pirq-43, virq-43 = ACCI: shared driver
     *  vimm-0, pirq-44, virq-44 = KMI: shared driver
     *  vimm-0, pirq-45, virq-45 = KMI: shared driver
     *  vimm-0, pirq-47, virq-47 = SMSC 91C111, Ethernet - etc0
     *  vimm-0, pirq-41, virq-41 = MCI - pl180
     *  vimm-0, pirq-42, virq-42 = MCI - pl180
     */
    DECLARE_VIRQMAP(_guest_virqmap, 0, 1, 1);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 16, 16);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 17, 17);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 18, 18);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 19, 19);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 31, 31);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 32, 32);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 33, 33);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 34, 34);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 35, 35);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 36, 36);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 37, 38);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 38, 37);
    DECLARE_VIRQMAP(_guest_virqmap, 1, 39, 37);
    DECLARE_VIRQMAP(_guest_virqmap, 2, 40, 37);
    DECLARE_VIRQMAP(_guest_virqmap, 3, 48, 37);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 41, 41);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 42, 42);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 43, 43);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 44, 44);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 45, 45);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 46, 46);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 47, 47);
    DECLARE_VIRQMAP(_guest_virqmap, 0, 69, 69);
}

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

    // TODO(casionwoo) : Move stage1_mm_init from arch/arm/main.c to here

    /* Initialize PIRQ to VIRQ mapping */
    setup_interrupt();
    /* Initialize Interrupt Management */
    if (interrupt_init(_guest_virqmap))
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

