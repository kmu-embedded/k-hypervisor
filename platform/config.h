#ifdef CONFIG_ARCH_VERSATILE
#include "./rtsm/config.h"
#elif CONFIG_RMOBILE
#include "./lager/config.h"
#elif CONFIG_ARCH_EXYNOS
#include "./odroidxu/config.h"
#elif CONFIG_ARCH_SUNXI
#include "./bananapi/config.h"
#endif
