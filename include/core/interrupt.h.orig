#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "hvmm_types.h"

#define HOST_IRQ 0
#define GUEST_IRQ 1
#define GUEST_IRQ_ENABLE 1
#define GUEST_IRQ_DISABLE 0

#define INJECT_SW 0
#define INJECT_HW 1

/**
 * @breif   Saves a mapping information to find a virq for injection.
 *
 * We do not consider a sharing device that's why we save only one vmid.
 * @todo    (wonseok): need to change a structure when we support
 *                     a sharing device among guests.
 */
struct virqmap_entry {
    uint32_t enabled;   /**< virqmap enabled flag */
    uint32_t virq;      /**< Virtual interrupt nubmer */
    uint32_t pirq;      /**< Pysical interrupt nubmer */
};

struct guest_virqmap {
    vmid_t vmid;
    struct virqmap_entry map[1024];
};

typedef void (*interrupt_handler_t)(int irq, void *regs, void *pdata);

struct interrupt_ops {
    /** Initalize interrupt state */
    void (*init)(void);

    /** Enable interrupt */
    void (*enable)(uint32_t);

    /** Disable interrupt */
    void (*disable)(uint32_t);

    /** Cofigure interrupt */
    void (*configure)(uint32_t);

    /** End of interrupt */
    void (*end)(uint32_t);

    /** Inject to guest */
    void (*inject)(vmid_t, uint32_t, uint32_t, uint8_t);

    /** Control SGI */
    void (*sgi)(uint32_t/*cpumask*/, uint32_t);

    /** Save inetrrupt state */
    void (*save)(vmid_t vmid);

    /** Restore interrupt state */
    void (*restore)(vmid_t vmid);

    /** Dump state of the interrupt */
    void (*dump)(void);
};

struct interrupt_module {
    /** tag must be initialized to HAL_TAG */
    uint32_t tag;

    /**
     * Version of the module-specific device API. This value is used by
     * the derived-module user to manage different device implementations.
     * The user who uses this module is responsible for checking
     * the module_api_version and device version fields to ensure that
     * the user is capable of communicating with the specific module
     * implementation.
     *
     */
    uint32_t version;

    /** Identifier of module */
    const char *id;

    /** Name of this module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Interrupt Operation */
    struct interrupt_ops *host_ops;
    struct interrupt_ops *guest_ops;
};

extern struct interrupt_module _interrupt_module;

/**
 * @brief   Initializes GIC, VGIC status.
 * <pre>
 * Initialization sequence.
 * 1. Routes IRQ/IFQ to Hyp Exception Vector.
 * 2. Initializes GIC Distributor & CPU Interface and enable them.
 * 3. Initializes and enable Virtual Interface Control.
 * 4. Initializes physical irq, virtual irq status table, and
 *    registers injection callback function that is called
 *    when flushs virtual irq.
 * </pre>
 * @return  If all initialization is success, then returns "success"
 *          otherwise returns "unknown error"
 */

void interrupt_init();
void register_irq_handler(uint32_t irq, interrupt_handler_t handler);
void interrupt_host_enable(uint32_t irq);
void interrupt_host_disable(uint32_t irq);
void interrupt_host_configure(uint32_t irq);
void interrupt_guest_inject(vmid_t vmid, uint32_t virq, uint32_t pirq,
                uint8_t hw);
void interrupt_guest_enable(vmid_t vmid, uint32_t irq);
void interrupt_guest_disable(vmid_t vmid, uint32_t irq);
// void interrupt_save(vmid_t vmid);
// void interrupt_restore(vmid_t vmid);
void interrupt_service_routine(int irq, void *current_regs, void *pdata);
const int32_t interrupt_check_guest_irq(uint32_t pirq);
const uint32_t interrupt_pirq_to_virq(vmid_t vmid, uint32_t pirq);
const uint32_t interrupt_virq_to_pirq(vmid_t vmid, uint32_t virq);
const uint32_t interrupt_pirq_to_enabled_virq(vmid_t vmid, uint32_t pirq);

#endif
