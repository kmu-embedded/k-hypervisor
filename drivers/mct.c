#include <drivers/mct.h>
#include <io.h>
#include <stdint.h>
#include "../include/arch/armv7/local_irq.h"

static inline uint32_t mct_read(uint32_t offset)
{
    return readl((void *) EXYNOS5_MCT_BASE + offset);
}

static void mct_write(uint32_t value, uint32_t offset)
{
    uint32_t addr;
    uint32_t mask;

    writel(value, (EXYNOS5_MCT_BASE + offset));
    switch (offset) {
    case EXYNOS5_MCT_G_TCON:
        addr = EXYNOS5_MCT_G_WSTAT;
        mask = 1 << 16; /* G_TCON write status */
        break;
    case EXYNOS5_MCT_G_CNT_L:
        addr = EXYNOS5_MCT_G_CNT_WSTAT;
        mask = 1 << 0;  /* G_CNT_L write status */
        break;
    case EXYNOS5_MCT_G_CNT_U:
        addr = EXYNOS5_MCT_G_CNT_WSTAT;
        mask = 1 << 1;  /* G_CNT_U write status */
        break;
    default:
        return;
    }
    writel(mask, EXYNOS5_MCT_BASE + addr);
}

void mct_init(void)
{
    uint32_t reg;

    mct_write(0, EXYNOS5_MCT_G_CNT_L);
    mct_write(0, EXYNOS5_MCT_G_CNT_U);
    reg = mct_read(EXYNOS5_MCT_G_TCON);
    reg |= MCT_G_TCON_START;
    mct_write(reg, EXYNOS5_MCT_G_TCON);
}
