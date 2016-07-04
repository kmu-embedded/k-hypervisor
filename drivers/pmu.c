#include <drivers/pmu.h>
#include <io.h>
#include <arch/armv7/smp.h>
#include <stdio.h>

unsigned int cpu_addr[4] = { 0x100100, 0x200200, 0x400400, 0x800800 };
unsigned int cpu_value[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };
unsigned int cpu_clear[8] = { 0x10, 0x14, 0x18, 0x1c, 0x0, 0x4, 0x8, 0xc};

void pmu_init()
{
    unsigned int i, value;


    writel(0xff0, EXYNOS5422_CENTRAL_SEQ_OPTION);


    value = readl(EXYNOS5422_ARM_L2_OPTION);
    value &= ~EXYNOS5_USE_RETENTION;
    writel(value, EXYNOS5422_ARM_L2_OPTION);

    value = readl(EXYNOS5422_KFC_L2_OPTION);
    value &= ~EXYNOS5_USE_RETENTION;
    writel(value, EXYNOS5422_KFC_L2_OPTION);

    writel(0x3, EXYNOS5422_PMU_SPARE3);

    value = readl(EXYNOS5422_LPI_MASK0);
    value |= EXYNOS5422_ATB_ISP_ARM | EXYNOS5422_DIS;
    writel(value, EXYNOS5422_LPI_MASK0);

    value = readl(EXYNOS5422_LPI_MASK1);
    value |= EXYNOS5422_ATB_KFC;
    writel(value, EXYNOS5422_LPI_MASK1);

    value = readl(EXYNOS5422_ARM_COMMON_OPTION);
    value |= EXYNOS5_SKIP_DEACTIVATE_ACEACP_IN_PWDN;
    writel(value, EXYNOS5422_ARM_COMMON_OPTION);

    value = readl(EXYNOS5422_PS_HOLD_CONTROL);
    value |= EXYNOS_PS_HOLD_OUTPUT_HIGH;
    writel(value, EXYNOS_PS_HOLD_CONTROL);

    value = readl(EXYNOS5422_PS_HOLD_CONTROL);
    value |= EXYNOS_PS_HOLD_EN;
    writel(value, EXYNOS5422_PS_HOLD_CONTROL);

    writel(0xf, EXYNOS5422_LOGIC_RESET_DURATION3);

    writel(0xf, EXYNOS5422_ARM_INTR_SPREAD_ENABLE);
    writel(0xf, EXYNOS5422_ARM_INTR_SPREAD_USE_STANDBYWFI);
    writel(0x1, EXYNOS5422_UP_SCHEDULER);

    value = readl(EXYNOS5422_ARM_COMMON_OPTION);
    value |= (1 << 30) | (1 << 29) | (1 << 9);
    writel(value, EXYNOS5422_ARM_COMMON_OPTION);

    for(i=0; i<8; i++) {
        value = readl(EXYNOS_ARM_CORE_OPTION(i));
        value &= ~EXYNOS_USE_DELAYED_RESET_ASSERTION;
        writel(value, EXYNOS_ARM_CORE_OPTION(i));
    }

    value = readl(EXYNOS5422_ARM_COMMON_OPTION);
    value |= EXYNOS_USE_DELAYED_RESET_ASSERTION;
    writel(value, EXYNOS5422_ARM_COMMON_OPTION);

    value = readl(EXYNOS5422_KFC_COMMON_OPTION);
    value |= EXYNOS_USE_DELAYED_RESET_ASSERTION;
    writel(value, EXYNOS5422_KFC_COMMON_OPTION);

    for(i=0; i<8; i++) {
        value = readl(EXYNOS_ARM_CORE_OPTION(i));
        value &= ~EXYNOS_ENABLE_AUTOMATIC_WAKEUP;
        writel(value, EXYNOS_ARM_CORE_OPTION(i));
    }

    for(i=0; i<8; i++) {
        value = readl(EXYNOS_ARM_CORE_OPTION(i));
        value &= ~EXYNOS5_USE_SC_COUNTER;
        value |= EXYNOS5_USE_SC_FEEDBACK;
        writel(value, EXYNOS_ARM_CORE_OPTION(i));
    }

    value = readl(EXYNOS5422_ARM_COMMON_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_ARM_COMMON_OPTION);

    value = readl(EXYNOS5422_KFC_COMMON_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_KFC_COMMON_OPTION);

    value = readl(EXYNOS5422_SCALER_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_SCALER_OPTION);

    value = readl(EXYNOS5422_ISP_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_ISP_OPTION);

    value = readl(EXYNOS5422_MFC_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_MFC_OPTION);

    value = readl(EXYNOS5422_G3D_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_G3D_OPTION);

    value = readl(EXYNOS5422_DISP1_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_DISP1_OPTION);

    value = readl(EXYNOS5422_MAU_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_MAU_OPTION);

    value = readl(EXYNOS5422_G2D_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_G2D_OPTION);

    value = readl(EXYNOS5422_MSC_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_MSC_OPTION);

    value = readl(EXYNOS5422_TOP_PWR_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_TOP_PWR_OPTION);

    value = readl(EXYNOS5422_TOP_PWR_COREBLK_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_TOP_PWR_COREBLK_OPTION);

    value = readl(EXYNOS5422_CAM_OPTION);
    value &= ~EXYNOS5_USE_SC_COUNTER;
    value |= EXYNOS5_USE_SC_FEEDBACK;
    writel(value, EXYNOS5422_CAM_OPTION);
}

void set_boot_addr()
{
    unsigned int boot_reg = ns_base + 0x1c;
    //void *boot_reg = ns_base + 0x1c;
    writel(0x70000000, boot_reg);
}

static int power_state(unsigned int cpu)
{
    unsigned int val;
    val = readl(EXYNOS_ARM_CORE_STATUS(cpu_value[cpu]));
    return val == EXYNOS_CORE_LOCAL_PWR_EN;
}

static void power_up(unsigned int cpu)
{
    writel(EXYNOS_CORE_LOCAL_PWR_EN, EXYNOS_ARM_CORE_CONFIGURATION(cpu_value[cpu]));
}

#include <core/timer.h>
void udelay(uint32_t usec)
{
    uint64_t deadline = timer_get_timenow() + 1000 * usec;
    while (timer_get_timenow() - deadline < 0)
        ;
    dsb();
    isb();
}

static int power_up_cpu(unsigned int cpu)
{
    uint32_t timeout, val;

//    cpu = (cpu + 4) % 8;
    if(!power_state(cpu)) {
        power_up(cpu);
        timeout = 10;

        while(power_state(cpu) != EXYNOS_CORE_LOCAL_PWR_EN)
        {
            if (timeout-- == 0)
                break;
        }

        if (timeout == 0)
        {
            printf("pcpu[%d] power enable failed\n", cpu);
            return -1;
        }
    }

//    cpu = (cpu + 4) % 8;
    if(cpu < 4) {
        while(!readl(EXYNOS_PMU_SPARE2))
            udelay(10);

        udelay(10);

        val = ( (1 << 20) | (1 << 8)) << cpu;
        writel(val, EXYNOS_SWRESET);
    }
    return 0;
}

void boot_secondary(unsigned int cpu)
{
    int ret;
    printf("%s %d: cpu[%d]\n", __func__, __LINE__, cpu);
    ret = power_up_cpu(cpu);
    smp_rmb();
    dsb_sev();
    if (ret == -1) printf("%s %d: cpu[%d] power enable failed\n", __func__, __LINE__, cpu);
}

#define HOTPLUG         (1 << 2)
void init_secondary(unsigned int cpu)
{
    unsigned int tmp;
    unsigned int addr = 0x02073028 + cpu_clear[cpu];
    //unsigned int addr = 0x02020028 + cpu_clear[cpu];

    tmp = readl(addr);
    tmp |= EXYNOS_CORE_LOCAL_PWR_EN;
    tmp &= ~HOTPLUG;

    writel(tmp, addr);
}
