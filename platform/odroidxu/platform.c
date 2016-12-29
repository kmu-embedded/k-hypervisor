#include "platform.h"
#include <stdint.h>
#include <serial.h>

// TODO(wonseok): moved header files from arch/arm to proper dir.
#include "../../arch/arm/paging.h"

#include <size.h>
#include <arch/gic_regs.h>

#include <arch/armv7.h>
#include <stdio.h>

#include <config.h>

void platform_init()
{
    uint32_t gic_base = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);

    gic_base = CFG_GIC_BASE_PA;
    paging_add_mapping(gic_base + GICD_OFFSET, gic_base + GICD_OFFSET, MT_DEVICE, SZ_4K);
    paging_add_mapping(gic_base + GICC_OFFSET, gic_base + GICC_OFFSET, MT_DEVICE, SZ_8K);

    paging_add_mapping(gic_base + GICH_OFFSET, gic_base + GICH_OFFSET, MT_DEVICE, SZ_4K);
    paging_add_mapping(gic_base + GICV_OFFSET, gic_base + GICV_OFFSET, MT_DEVICE, SZ_8K);

    // add mapping for serial devices
    paging_add_mapping(0x12c20000, 0x12c20000, MT_DEVICE, SZ_64K);
    // add mapping for timer devices
    paging_add_mapping(0x101c0000, 0x101c0000, MT_DEVICE, SZ_4K);

    paging_add_mapping(0x02073000, 0x02073000, MT_DEVICE, SZ_4K);
    paging_add_mapping(0x10040000, 0x10040000, MT_DEVICE, SZ_64K);

    paging_add_mapping(CFG_HYP_START_ADDRESS, CFG_HYP_START_ADDRESS, MT_WRITEBACK_RW_ALLOC, SZ_256M);
}

void console_init()
{
    // TODO(wonseok): add general initialization for console devices.
    serial_init();
}

#include <drivers/mct.h>
#include <drivers/pmu.h>
#include <core/timer.h>

void dev_init()
{
//#ifdef CONFIG_MCT
#if 1
    mct_init();
#endif
//#ifdef CONFIG_PMU
#if 1
    pmu_init();
    // set boot_addr at ns_base
#endif
	timer_hw_init(NS_PL2_PTIMER_IRQ);
    // init .text.dev section like vdev.
}
