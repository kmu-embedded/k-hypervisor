#include "arch_init.h"

#include <stdint.h>
#include <libc_init.h>  //for malloc_init
#include <arch/armv7/smp.h>    //smp_processor_id()
#include <assert.h>
#include <asm/asm.h>

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")

#define disable_irq()       asm volatile(\
                            "cpsid  aif\n\t" : : : "memory", "cc")

extern uint32_t __hvc_vector;

void SECTION(".init.arch") cpu_init()
{
    /* Read current processor id. */
    uint8_t cpu_id = smp_processor_id();

    // TODO(wonseok): disable irq.
    disable_irq();

    if (cpu_id == 0) {
        /* The below sections should be initialized at once. */
        // TODO(wonseok): Initialize BSS section.
        // BSS section initialization in ASM
    }

    // Set vector table base address in HVBAR
    write_hvbar(&__hvc_vector);
    assert(read_hvbar() == &__hvc_vector);

    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

#include "gic-v2.h"
#include "vgic.h"

void SECTION(".init.arch") irq_init()
{
    // TODO(casionwoo): add a init function of irq handler table for hypervisor.
    gic_init();
    write_hcr(HCR_IMO | HCR_FMO);
    vgic_init();
    vgic_enable(1);
    virq_table_init();
}

#include <mm.h>

void SECTION(".init.arch") mm_init()
{
    uint8_t cpu_id = smp_processor_id();

    enable_mmu();

    if (cpu_id == 0) {
        __malloc_init(); // Do not need to initialize for all cores.
    }
}
