#include "platform.h"
#include <stdint.h>
#include <serial.h>

// TODO(wonseok): moved header files from arch/arm to proper dir.
#include "../../arch/arm/paging.h"

#include <size.h>
#include <arch/armv7.h>
#include <stdio.h>

#include <config.h>
#include <arch/irq.h>
#include "../../include/arch/gicv2_bit.h"

void platform_init()
{
    uint32_t gic_base = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);

    gic_base = CFG_GIC_BASE_PA;
    paging_add_mapping(gic_base + GICD_OFFSET, gic_base + GICD_OFFSET, MT_DEVICE, SZ_4K);
    paging_add_mapping(gic_base + GICC_OFFSET, gic_base + GICC_OFFSET, MT_DEVICE, SZ_8K);

    paging_add_mapping(gic_base + GICH_OFFSET, gic_base + GICH_OFFSET, MT_DEVICE, SZ_4K);
    paging_add_mapping(gic_base + GICV_OFFSET, gic_base + GICV_OFFSET, MT_DEVICE, SZ_8K);

    // add mapping for serial devices
    paging_add_mapping(0xE6E60000, 0xE6E60000, MT_DEVICE, SZ_64K);

    paging_add_mapping(CFG_HYP_START_ADDRESS, CFG_HYP_START_ADDRESS, MT_WRITEBACK_RW_ALLOC, SZ_128M);
}

void console_init()
{
    // TODO(wonseok): add general initialization for console devices.
    serial_init();
}
#include <core/timer.h>

void dev_init()
{
    // init .text.dev section like vdev.
    timer_hw_init(NS_PL2_PTIMER_IRQ);
}
