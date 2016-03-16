#include "platform.h"
#include <stdio.h>
#include <stdint.h>
#include <libc_init.h>
#include <drivers/pl01x.h>

// TODO(wonseok): moved header files from arch/arm to proper dir.
#include "../../arch/arm/mm.h"
#include "../../arch/arm/lpae.h"

#include <size.h>
#include <arch/gic_regs.h>
#include <asm/asm.h>

#include <arch/armv7.h>

#include "rtsm-config.h"
void SECTION(".init.platform") platform_init()
{
    uint32_t gic_base = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);
    if (gic_base == 0x0) {
        gic_base = 0x2C000000;
    }

    add_hyp_pgentry(gic_base + GICD_OFFSET, gic_base + GICD_OFFSET, MT_DEVICE, SZ_4K);
    add_hyp_pgentry(gic_base + GICC_OFFSET, gic_base + GICC_OFFSET, MT_DEVICE, SZ_8K);
    add_hyp_pgentry(gic_base + GICH_OFFSET, gic_base + GICH_OFFSET, MT_DEVICE, SZ_4K);
    add_hyp_pgentry(gic_base + GICV_OFFSET, gic_base + GICV_OFFSET, MT_DEVICE, SZ_8K);

    // add mapping for serial devices
    add_hyp_pgentry(0x1C090000, 0x1C090000, MT_DEVICE, SZ_4K);
    add_hyp_pgentry(0x1C0A0000, 0x1C0A0000, MT_DEVICE, SZ_4K);
    add_hyp_pgentry(0x1C0B0000, 0x1C0B0000, MT_DEVICE, SZ_4K);
    add_hyp_pgentry(0x1C0C0000, 0x1C0C0000, MT_DEVICE, SZ_4K);

    add_hyp_pgentry(0xF0000000, CFG_HYP_START_ADDRESS, MT_WRITEBACK_RW_ALLOC, SZ_256M);
    add_hyp_pgentry(0x80000000, 0x80000000, MT_WRITEBACK_RW_ALLOC, SZ_256K);
    add_hyp_pgentry(0x90000000, 0x90000000, MT_WRITEBACK_RW_ALLOC, SZ_256K);
    add_hyp_pgentry(0xA0000000, 0xA0000000, MT_WRITEBACK_RW_ALLOC, SZ_256K);
}

void SECTION(".init.platform") console_init()
{
    // TODO(wonseok): add general initialization for console devices.
    pl01x_init(115200, 24000000);
}

void SECTION(".init.platform") dev_init()
{
    // init .text.dev section like vdev.
}
