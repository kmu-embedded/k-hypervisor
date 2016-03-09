#include <core/vm/virq.h>
#include <debug_print.h>
#include <rtsm-config.h>

#define DECLARE_VIRQ(name, _pirq, _virq) \
    do {                                 \
        name->map[_pirq].virq = _virq;   \
        name->map[_virq].pirq = _pirq;   \
    } while (0)

#define DECLARE_VIRQMAP(name, id, _pirq, _virq) \
    do {                                        \
        name[id].map[_pirq].virq = _virq;       \
        name[id].map[_virq].pirq = _pirq;       \
    } while (0)

static struct guest_virqmap _guest_virqmap[NUM_GUESTS_STATIC];

/* TODO(igxactly): move code from guest_interrupt_init --> virq_setup, virq_create */
void virq_create(struct virq *virq, vmid_t vmid)
{
    /* TODO(igkang): something like virq_setup will be called here
     *  to load configurations from DTB or config data */

    /* Initialize PIRQ to VIRQ mapping */
    int j;
    struct virqmap_entry *map;

    map = _guest_virqmap[vmid].map;
    for (j = 0; j < MAX_IRQS; j++) {
        map[j].enabled = GUEST_IRQ_DISABLE;
        map[j].virq = VIRQ_INVALID;
        map[j].pirq = PIRQ_INVALID;
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

    switch (vmid) {
    case 0:
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
        DECLARE_VIRQMAP(_guest_virqmap, 0, 38, 37);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 41, 41);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 42, 42);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 43, 43);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 44, 44);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 45, 45);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 46, 46);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 47, 47);
        DECLARE_VIRQMAP(_guest_virqmap, 0, 69, 69);
        break;

    case 1:
        DECLARE_VIRQMAP(_guest_virqmap, 1, 1, 1);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 16, 16);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 17, 17);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 18, 18);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 19, 19);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 31, 31);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 32, 32);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 33, 33);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 34, 34);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 35, 35);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 36, 36);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 39, 37);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 41, 41);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 42, 42);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 43, 43);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 44, 44);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 45, 45);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 46, 46);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 47, 47);
        DECLARE_VIRQMAP(_guest_virqmap, 1, 69, 69);
        break;

    case 2:
        DECLARE_VIRQMAP(_guest_virqmap, 2, 1, 1);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 16, 16);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 17, 17);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 18, 18);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 19, 19);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 31, 31);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 32, 32);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 33, 33);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 34, 34);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 35, 35);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 36, 36);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 40, 37);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 41, 41);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 42, 42);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 43, 43);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 44, 44);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 45, 45);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 46, 46);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 47, 47);
        DECLARE_VIRQMAP(_guest_virqmap, 2, 69, 69);
        break;

    default:
        debug_print("virq_create error!\n");
        break;
    }
}

hvmm_status_t virq_init(struct virq *virq, vmid_t vmid)
{
    /* guest_virqmap initialization */
    virq->guest_virqmap = &_guest_virqmap[vmid];

    /* vgic_status initialization */
    return vgic_init_status(&virq->vgic_status);
}

hvmm_status_t virq_save(struct virq *virq)
{
    return vgic_save_status(&virq->vgic_status);
}

hvmm_status_t virq_restore(struct virq *virq, vmid_t vmid)
{
    return vgic_restore_status(&virq->vgic_status, vmid);
}
