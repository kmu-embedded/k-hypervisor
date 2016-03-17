#include <core/vm/vmem.h>
#include <debug.h>
#include <config.h>
#include <arch/armv7.h>
#include <size.h>
#include "../../arch/arm/paging.h"

struct memdesc_t {
    char *label;
    uint64_t ipa;
    uint64_t pa;
    uint32_t size;
    uint8_t attr;
};

// TODO(casionwoo) : These memory mapping variables should be removed after DTB implementation
enum memattr {
    MEMATTR_STRONGLY_ORDERED        = 0x0,  // 00_00
    MEMATTR_DEVICE_MEMORY           = 0x1,  // 00_01
    MEMATTR_NORMAL_NON_CACHEABLE    = 0x5,  // 01_01
    MEMATTR_NORMAL_WT_CACHEABLE     = 0xA,  // 10_10
    MEMATTR_NORMAL_WB_CACHEABLE     = 0xF,  // 11_11
};

struct memdesc_t vm_md_empty[] = {
    { 0, 0, 0, 0,  0 },
};

/*  label, ipa, pa, size, attr */
struct memdesc_t vm0_device_md[] = {
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
    {
        "gicc", CFG_GIC_BASE_PA | 0x2000,
        CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY
    },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

struct memdesc_t vm1_device_md[] = {
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
    {
        "gicc", CFG_GIC_BASE_PA | 0x2000,
        CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY
    },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

struct memdesc_t vm2_device_md[] = {
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
    {
        "gicc", CFG_GIC_BASE_PA | 0x2000,
        CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY
    },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MEMATTR_DEVICE_MEMORY },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MEMATTR_DEVICE_MEMORY },
    { 0, 0, 0, 0, 0 }
};

/**
 * @brief Memory map for VM0 image .
 */
struct memdesc_t vm0_memory_md[] = {
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
    {0, 0, 0, 0,  0},
};

/**
 * @brief Memory map for VM1 image.
 */
struct memdesc_t vm1_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
    {0, 0, 0, 0,  0},
};

struct memdesc_t vm2_memory_md[] = {
    /* 256MB */
    {"start", CFG_GUEST_ATAGS_START_ADDRESS, 0, 0x10000000, MEMATTR_NORMAL_WB_CACHEABLE },
    {0, 0, 0, 0,  0},
};

/* Memory Map for VM 0 */
struct memdesc_t *vm0_mdlist[] = {
    vm0_device_md,   /* 0x0000_0000 */
    vm_md_empty,     /* 0x4000_0000 */
    vm0_memory_md,
    vm_md_empty,     /* 0xC000_0000 PA:0x40000000*/
    0
};

/* Memory Map for VM 1 */
struct memdesc_t *vm1_mdlist[] = {
    vm1_device_md,
    vm_md_empty,
    vm1_memory_md,
    vm_md_empty,
    0
};

/* Memory Map for VM 2 */
struct memdesc_t *vm2_mdlist[] = {
    vm2_device_md,
    vm_md_empty,
    vm2_memory_md,
    vm_md_empty,
    0
};

void __set_memmap(struct vmem *vmem, vmid_t vmid)
{
    if (vmid == 0) {
        vm0_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST0_ATAGS_OFFSET);
    } else if (vmid == 1) {
        vm1_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST1_ATAGS_OFFSET);
    } else if (vmid == 2) {
        vm2_memory_md[0].pa = (uint64_t)((uint32_t) CFG_MEMMAP_GUEST2_ATAGS_OFFSET);
    }

    if (vmid == 0) {
        vmem->memmap = vm0_mdlist;
    } else if (vmid == 1) {
        vmem->memmap = vm1_mdlist;
    } else if (vmid == 2) {
        vmem->memmap = vm2_mdlist;
    }
}

extern uint32_t __VM_PGTABLE;
extern uint32_t __END_VM_PGTABLE;
#define PGTABLE_SIZE    0x805000

#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFFFFFULL

void vmem_create(struct vmem *vmem, vmid_t vmid)
{

    vmem->base = (uint32_t) &__VM_PGTABLE + (PGTABLE_SIZE << (vmid - 1));
    paging_create(vmem->base);

    vmem->vttbr = ((uint64_t) vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
    vmem->vttbr &= ~(VTTBR_BADDR_MASK);
    vmem->vttbr |= (uint32_t) vmem->base & VTTBR_BADDR_MASK;

    // TODO(wonseok): is vmem owned memory mapping?
    __set_memmap(vmem, vmid);
}

hvmm_status_t vmem_init(struct vmem *vmem)
{
    int i, j;

    struct memdesc_t *map;
    for (i = 0; vmem->memmap[i]; i++) {
        if (vmem->memmap[i]->label == 0) {
            continue;
        }

        j = 0;
        map = vmem->memmap[i];
        while (map[j].label != 0) {
            paging_add_ipa_mapping(vmem->base, map[j].ipa, map[j].pa, map[j].attr, map[j].size);
            j++;
        }
    }

    vmem->vtcr = (VTCR_SL0_FIRST_LEVEL << VTCR_SL0_BIT);
    vmem->vtcr |= (WRITEBACK_CACHEABLE << VTCR_ORGN0_BIT);
    vmem->vtcr |= (WRITEBACK_CACHEABLE << VTCR_IRGN0_BIT);
    write_vtcr(vmem->vtcr);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vmem_save(void)
{
    write_hcr(read_hcr() & ~(0x1));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vmem_restore(struct vmem *vmem)
{
    write_vttbr(vmem->vttbr);

    write_hcr(read_hcr() | (0x1));

    return HVMM_STATUS_SUCCESS;
}

