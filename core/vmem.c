#include <vmem.h>
#include <gic_regs.h>

// TODO(casionwoo) : These memory mapping variables should be removed after DTB implementation

extern uint32_t _guest0_bin_start;
extern uint32_t _guest1_bin_start;

struct memmap_desc vm_md_empty[] = {
    { 0, 0, 0, 0,  0 },
};
struct memmap_desc vm0_device_md[] = {
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
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, MEMATTR_DM },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MEMATTR_DM },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MEMATTR_DM },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MEMATTR_DM },
    { "gicc", CFG_GIC_BASE_PA | GIC_OFFSET_GICC,
              CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, MEMATTR_DM },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DM },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DM },
    { 0, 0, 0, 0, 0 }
};

struct memmap_desc vm1_device_md[] = {
    { "uart", 0x1C090000, 0x1C0B0000, SZ_4K, MEMATTR_DM },
    //{ "sp804", 0x1C110000, 0x1C120000, SZ_4K, MEMATTR_DM },
    { "gicc", 0x2C000000 | GIC_OFFSET_GICC,
       CFG_GIC_BASE_PA | GIC_OFFSET_GICVI, SZ_8K, MEMATTR_DM },
    {0, 0, 0, 0, 0}
};

/**
 * @brief Memory map for VM0 image .
 */
struct memmap_desc vm0_memory_md[] = {
    {"start", CFG_GUEST_START_ADDRESS, 0, 0x40000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for VM1 image.
 */
struct memmap_desc vm1_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_START_ADDRESS, 0, 0x10000000,
     MEMATTR_NORMAL_OWB | MEMATTR_NORMAL_IWB
    },
    {0, 0, 0, 0,  0},
};

/* Memory Map for VM 0 */
struct memmap_desc *vm0_mdlist[] = {
    vm0_device_md,   /* 0x0000_0000 */
    vm_md_empty,     /* 0x4000_0000 */
    vm0_memory_md,
    vm_md_empty,     /* 0xC000_0000 PA:0x40000000*/
    0
};

/* Memory Map for VM 1 */
struct memmap_desc *vm1_mdlist[] = {
    vm1_device_md,
    vm_md_empty,
    vm1_memory_md,
    vm_md_empty,
    0
};

void vmem_setup()
{
    /* Pagetable Initialization */
    memory_hw_setup();
}

void vmem_create(struct vmem *vmem, vmid_t vmid)
{
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */

    if(vmid == 0) {
        vm0_memory_md[0].pa = (uint64_t)((uint32_t) &_guest0_bin_start);
    }else {
        vm1_memory_md[0].pa = (uint64_t)((uint32_t) &_guest1_bin_start);
    }

    vmem->memmap = (vmid == 0) ? vm0_mdlist : vm1_mdlist;
}

hvmm_status_t vmem_init(struct vmem *vmem, vmid_t vmid)
{
    return memory_hw_init(vmem->memmap, &vmem->pgtable_base, vmid);
}

hvmm_status_t vmem_save(void)
{
    return memory_hw_save();
}

hvmm_status_t vmem_restore(struct vmem *vmem, vmid_t vmid)
{
    return memory_hw_restore(vmid, &vmem->pgtable_base);
}

