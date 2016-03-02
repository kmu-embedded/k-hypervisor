#include <core/vm/vmem.h>
#include <debug_print.h>
#include <rtsm-config.h>

// TODO(casionwoo) : These memory mapping variables should be removed after DTB implementation

//extern uint32_t _guest0_bin_start;
//extern uint32_t _guest1_bin_start;
//extern uint32_t _guest2_bin_start;
//
struct memmap_desc vm_md_empty[] = {
    { 0, 0, 0, 0,  0 },
};

/*  label, ipa, pa, size, attr */
struct memmap_desc vm0_device_md[] = {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "kmi", 0x1C060000, 0x1C060000,  SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "kmi2", 0x1C070000, 0x1C070000, SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial0", 0x1C090000, 0x1C0A0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial1", 0x1C0A0000, 0x1C090000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "gicc", CFG_GIC_BASE_PA | 0x2000,
            CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

struct memmap_desc vm1_device_md[] = {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "kmi", 0x1C060000, 0x1C060000,  SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "kmi2", 0x1C070000, 0x1C070000, SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial0", 0x1C090000, 0x1C0B0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial1", 0x1C0A0000, 0x1C090000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "gicc", CFG_GIC_BASE_PA | 0x2000,
            CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

struct memmap_desc vm2_device_md[] = {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "kmi", 0x1C060000, 0x1C060000,  SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "kmi2", 0x1C070000, 0x1C070000, SZ_64K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial0", 0x1C090000, 0x1C0C0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial1", 0x1C0A0000, 0x1C090000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MEMATTR_DEVICE_MEMORY },
    { "gicc", CFG_GIC_BASE_PA | 0x2000,
            CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

/**
 * @brief Memory map for VM0 image .
 */
struct memmap_desc vm0_memory_md[] = {
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for VM1 image.
 */
struct memmap_desc vm1_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
    {0, 0, 0, 0,  0},
};

struct memmap_desc vm2_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
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

/* Memory Map for VM 2 */
struct memmap_desc *vm2_mdlist[] = {
    vm2_device_md,
    vm_md_empty,
    vm2_memory_md,
    vm_md_empty,
    0
};

void set_memmap(struct vmem *vmem, vmid_t vmid)
{
    /*
     * VA: 0x00000000 ~ 0x3FFFFFFF,   1GB
     * PA: 0xA0000000 ~ 0xDFFFFFFF    guest_bin_start
     * PA: 0xB0000000 ~ 0xEFFFFFFF    guest2_bin_start
     */

    if(vmid == 0) {
        vm0_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST0_ATAGS_OFFSET);
    }else if(vmid == 1) {
        vm1_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST1_ATAGS_OFFSET);
    }else if(vmid == 2) {
        vm2_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST2_ATAGS_OFFSET);
    }

    if(vmid == 0) {
        vmem->memmap = vm0_mdlist;
    }else if(vmid == 1) {
        vmem->memmap = vm1_mdlist;
    }else if(vmid == 2) {
        vmem->memmap = vm2_mdlist;
    }
}

void vmem_create(struct vmem *vmem, vmid_t vmid)
{
    /* Assign memory mapping */
    set_memmap(vmem, vmid);
    /* Pagetable Initialization */
    memory_hw_create(vmem);
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

