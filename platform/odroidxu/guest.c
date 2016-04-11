#include <vm_map.h>
#include <size.h>

/*  label, ipa, pa, size, attr */
struct memdesc_t vm0_device_md[] = {
    { "v2m_serial0", 0x12C20000, 0x12C20000, SZ_64K, MEMATTR_DEVICE_MEMORY, 1 },
    { "gicd", CFG_GIC_BASE_PA | 0x1000, CFG_GIC_BASE_PA | 0x1000, SZ_4K, MEMATTR_DEVICE_MEMORY, 0 },
    { "gicc", CFG_GIC_BASE_PA | 0x2000, CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY, 1 },
    {"start", 0x40000000, 0x40000000, SZ_1M, MEMATTR_NORMAL_WB_CACHEABLE, 1 },
    { 0, 0, 0, 0, 0, 0 }
};

struct memdesc_t vm1_device_md[] = {
    { "v2m_serial0", 0x12C20000, 0x12C20000, SZ_64K, MEMATTR_DEVICE_MEMORY, 1 },
    { "gicd", CFG_GIC_BASE_PA | 0x1000, CFG_GIC_BASE_PA | 0x1000, SZ_4K, MEMATTR_DEVICE_MEMORY, 0 },
    { "gicc", CFG_GIC_BASE_PA | 0x2000, CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY, 1 },
    {"start", 0x40000000, 0x50000000, SZ_1M, MEMATTR_NORMAL_WB_CACHEABLE, 1 },
    { 0, 0, 0, 0, 0, 0 }
};

struct memdesc_t vm2_device_md[] = {
    { "v2m_serial0", 0x12C20000, 0x12C20000, SZ_64K, MEMATTR_DEVICE_MEMORY, 1 },
    { "gicd", CFG_GIC_BASE_PA | 0x1000, CFG_GIC_BASE_PA | 0x1000, SZ_4K, MEMATTR_DEVICE_MEMORY, 0 },
    { "gicc", CFG_GIC_BASE_PA | 0x2000, CFG_GIC_BASE_PA | 0x6000, SZ_8K, MEMATTR_DEVICE_MEMORY, 1 },
    {"start", 0x40000000, 0x60000000, SZ_1M, MEMATTR_NORMAL_WB_CACHEABLE, 1 },
    { 0, 0, 0, 0, 0, 0 }
};

struct memdesc_t *vm_mmap[NUM_GUESTS_STATIC];


void setup_vm_mmap(void)
{
    // TODO(wonseok): it will be replaced by DTB.
    vm_mmap[0] = vm0_device_md;
    vm_mmap[1] = vm1_device_md;
    vm_mmap[2] = vm2_device_md;
}
