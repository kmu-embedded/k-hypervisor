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

extern uint32_t __hvc_vector;

void SECTION(".init.arch") cpu_init()
{
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
    vgic_init();
    virq_table_init();
    vgic_enable(1);
    write_hcr(HCR_IMO | HCR_FMO);
}

#include <mm.h>

void SECTION(".init.arch") mm_init()
{
    set_hmair();
    set_htcr();
    set_httbr();
    enable_mmu();
}
