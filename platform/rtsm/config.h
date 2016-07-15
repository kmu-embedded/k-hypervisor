#ifndef __CFG_H__
#define __CFG_H__

#define MAHINE_TYPE             2272
#define CFG_CNTFRQ              100000000

#define HYP_STACK_SIZE          4096
#define NR_CPUS                 4
#define NUM_GUESTS_STATIC       3

#define BOOTABLE_CPUID         	(0 << 8)

// TODO(casionwoo) : CFG_VDEV_SERIAL setting should be moved to menuconfig
/*
    CFG_VDEV_SERIAL == 0 : Use vdev_serial
    CFG_VDEV_SERIAL == 1 : Not use vdev_serial
*/
#define CFG_VDEV_SERIAL     0

// TODO(casionwoo) : CONFIG_LOG_LEVEL setting should be moved to menuconfig
/*
    CONFIG_LOG_LEVEL == 0 : Normal printf
    CONFIG_LOG_LEVEL == 1 : CPUID prefix + Normal printf
    CONFIG_LOG_LEVEL == 2 : CPUID prefix + __func__, __LINE + Normal printf
*/
#define CONFIG_LOG_LEVEL    0

/*
 *  SOC param
 */
#define CFG_GIC_BASE_PA     0x2C000000
#define CFG_GICD_BASE_PA    0x2C001000
#define CFG_GICC_BASE_PA    0x2C002000

#define NSEC_DIV 1000000000
#define TIMER_RESOLUTION_NS (NSEC_DIV/CFG_CNTFRQ)

#define NUM_GUESTS_CPU0_STATIC       2
#define NUM_GUESTS_CPU1_STATIC       2

#define MAX_IRQS 1024
#define MAX_PPI_IRQS 32
#define MAX_SPI_IRQS (MAX_IRQS - 1024)

#define CFG_MEMMAP_PHYS_START      0x80000000
#define CFG_MEMMAP_PHYS_SIZE       0x7FFFFFFF
#define CFG_MEMMAP_PHYS_END        (CFG_MEMMAP_PHYS_START+CFG_MEMMAP_PHYS_SIZE)
#define CFG_MEMMAP_MON_OFFSET      0xF0000000

#define CFG_MEMMAP_GUEST_SIZE      0x10000000
#define CFG_MEMMAP_GUEST0_ATAGS_OFFSET      0x80000000
#define CFG_MEMMAP_GUEST0_OFFSET            0x80500000
#define CFG_MEMMAP_GUEST1_ATAGS_OFFSET      0x90000000
#define CFG_MEMMAP_GUEST1_OFFSET            0x90500000
#define CFG_MEMMAP_GUEST2_ATAGS_OFFSET      0xA0000000
#define CFG_MEMMAP_GUEST2_OFFSET            0xA0500000
#define CFG_MEMMAP_GUEST3_ATAGS_OFFSET      0xB0000000
#define CFG_MEMMAP_GUEST3_OFFSET            0xB0500000
#define CFG_GUEST_ATAGS_START_ADDRESS       0x80000000
#define CFG_GUEST_START_ADDRESS             0x80500000

#define CFG_HYP_START_ADDRESS      0xF0000000

#endif  /* RTSM_CFG_H */
