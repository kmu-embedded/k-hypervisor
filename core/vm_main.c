#include <vm_main.h>
#include <vm.h>
#include <gic_regs.h>
#include <core.h>
#include <scheduler.h>
#include <arch/arm/rtsm-config.h>
#include <version.h>
#include <hvmm_trace.h>

static vmid_t vm[NUM_GUESTS_STATIC];

extern uint32_t _guest0_bin_start;
extern uint32_t _guest0_bin_end;
extern uint32_t _guest1_bin_start;
extern uint32_t _guest1_bin_end;

/**
 * \defgroup Guest_memory_map_descriptor
 *
 * Descriptor setting order
 * - label
 * - Intermediate Physical Address (IPA)
 * - Physical Address (PA)
 * - Size of memory region
 * - Memory Attribute
 * @{
 */
static struct memmap_desc guest_md_empty[] = {
    {       0, 0, 0, 0,  0},
};

/*  label, ipa, pa, size, attr */
static struct memmap_desc guest0_device_md[] = {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, MEMATTR_DM },
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, MEMATTR_DM },
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, MEMATTR_DM },
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, MEMATTR_DM },
    { "kmi", 0x1C060000, 0x1C060000,  SZ_64K, MEMATTR_DM },
    { "kmi2", 0x1C070000, 0x1C070000, SZ_64K, MEMATTR_DM },
    { "v2m_serial0", 0x1C090000, 0x1C0A0000, SZ_4K, MEMATTR_DM },
    { "v2m_serial1", 0x1C0A0000, 0x1C090000, SZ_4K, MEMATTR_DM },
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, MEMATTR_DM },
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, MEMATTR_DM },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MEMATTR_DM },
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K,
            MEMATTR_DM },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MEMATTR_DM },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MEMATTR_DM },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
            CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K,
            MEMATTR_DM },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DM },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

static struct memmap_desc guest1_device_md[] = {
    { "uart", 0x1C090000, 0x1C0B0000, SZ_4K, MEMATTR_DM },
    //{ "sp804", 0x1C110000, 0x1C120000, SZ_4K, MEMATTR_DM },
    { "gicc", 0x2C000000 | GIC_OFFSET_GICC,
       CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, MEMATTR_DM },
    {0, 0, 0, 0, 0}
};

/**
 * @brief Memory map for guest 0.
 */
static struct memmap_desc guest0_memory_md[] = {
    {"start", CFG_GUEST_START_ADDRESS, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for guest 1.
 */
static struct memmap_desc guest1_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_START_ADDRESS, 0, 0x10000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/* Memory Map for Guest 0 */
static struct memmap_desc *guest0_mdlist[] = {
    guest0_device_md,   /* 0x0000_0000 */
    guest_md_empty,     /* 0x4000_0000 */
    guest0_memory_md,
    guest_md_empty,     /* 0xC000_0000 PA:0x40000000*/
    0
};

/* Memory Map for Guest 1 */
static struct memmap_desc *guest1_mdlist[] = {
    guest1_device_md,
    guest_md_empty,
    guest1_memory_md,
    guest_md_empty,
    0
};

void setup_memory()
{
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */
    guest0_memory_md[0].pa = (uint64_t)((uint32_t) &_guest0_bin_start);
    guest1_memory_md[0].pa = (uint64_t)((uint32_t) &_guest1_bin_start);
}

int main_cpu_init()
{
    int i = 0;

    printf("[%s : %d] Starting...Main CPU\n", __func__, __LINE__);

    /* HYP initialization */
    khypervisor_init();

    /* Print Banner */
    printf("%s", BANNER_STRING);

    /* Scheduler initialization */
    sched_init();

    /* Initialize vCPUs */
    setup_memory(); // FIXME(casionwoo) : This should be removed later
    vm_setup();
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        if ((vm[i] = vm_create(1)) == VM_CREATE_FAILED) {
            printf("vm_create(vm[%d]) is failed...\n", i);
        }

        /* FIXME(casionwoo) : This below memmap setting should be removed after DTB implementing */
        if (i == 0) {
            vm_find(i)->vmem.memmap = guest0_mdlist;
        }else {
            vm_find(i)->vmem.memmap = guest1_mdlist;
        }

        if (vm_init(vm[i]) != HALTED) {
            printf("vm_init(vm[%d]) is failed...\n", i);
        }
        if (vm_start(vm[i]) != RUNNING) {
            printf("vm_start(vm[%d]) is failed...\n", i);
        }
    }

    /* Switch to the first vcpu */
    guest_sched_start();

    /* The code flow must not reach here */
    printf("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();
}

int vm_main(void)
{
    main_cpu_init();

    return 0;
}
