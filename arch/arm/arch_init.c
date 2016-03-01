#include <arch_init.h>
#include <stdint.h>
#include <libc_init.h>  //for malloc_init
#include <smp.h>    //smp_processor_id()
#include <assert.h>

#define read_hvbar()        ({ uint32_t rval; asm volatile(\
                            "mrc     p15, 4, %0, c12, c0, 0\n\t" \
                            : "=r" (rval) : : "memory", "cc"); rval; })
#define write_hvbar(val)    asm volatile(\
                            "mcr     p15, 4, %0, c12, c0, 0\n\t" \
                            : : "r" ((val)) : "memory", "cc")

#define disable_irq()       asm volatile(\
                            "cpsid  aif\n\t" : : : "memory", "cc")

extern uint32_t __hvc_vector;

void cpu_init()
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

void irq_init()
{
    // TODO(casionwoo): add a init function of irq handler table for hypervisor.
    gic_init();
    write_hcr(HCR_IMO | HCR_FMO);
    vgic_init();
    vgic_enable(1);
    virq_table_init();
}

#include <mm.h>             //memmeap_desc
#include <size.h>           // SZ_4K
#include <lpae.h>           // MT_DEVICE

struct memmap_desc memdesc[] = {
    { "gicd", 0x2c001000, 0x2c001000, SZ_4K, MT_DEVICE },
    // TODO(wonseok): Size of GICC is 0x2000(SZ_8K).
    { "gicc", 0x2c002000, 0x2c002000, SZ_4K, MT_DEVICE },
    { "gicc", 0x2c003000, 0x2c003000, SZ_4K, MT_DEVICE },

    { "gich", 0x2c004000, 0x2c004000, SZ_4K, MT_DEVICE },
    { "gich", 0x2c005000, 0x2c005000, SZ_4K, MT_DEVICE },
    { "gicv", 0x2c006000, 0x2c006000, SZ_4K, MT_DEVICE },

    // TODO(casionwoo): Recude the size of atags.
    // FIXME(casionwoo): Make atags configurable.
    { "atags_0", 0x80000000, 0x80000000, 0x00500000, MT_WRITEBACK_RW_ALLOC },
    { "atags_1", 0x90000000, 0x90000000, 0x00500000, MT_WRITEBACK_RW_ALLOC },
    { "atags_2", 0xA0000000, 0xA0000000, 0x00500000, MT_WRITEBACK_RW_ALLOC },
    { 0, 0, 0, 0, 0 }
};

void mm_init()
{
    uint8_t cpu_id = smp_processor_id();
    /* mm_init() must execute for each CPUs. */
    // TODO(wonseok) split memory and devices from memdesc.
    set_pgtable(&memdesc);

    enable_mmu();

    if (cpu_id == 0) {
        __malloc_init(); // Do not need to initialize for all cores.
    }
}
