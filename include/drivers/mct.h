#ifndef __MCT__H__
#define __MCT__H__

#define EXYNOS5_MCT_BASE        0x101C0000
#define EXYNOS5_MCT_G_TCON      0x240
#define EXYNOS5_MCT_G_WSTAT     0x24C
#define EXYNOS5_MCT_G_CNT_WSTAT 0x110
#define EXYNOS5_MCT_G_CNT_L     0x100
#define EXYNOS5_MCT_G_CNT_U     0x104
#define MCT_G_TCON_START        (1 << 8)

void mct_init(void);
#endif
