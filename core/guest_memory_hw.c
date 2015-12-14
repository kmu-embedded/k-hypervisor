#include <stdio.h>
#include <armv7_p15.h>
#include <stdint.h>
#include <hvmm_trace.h>
#include <memory.h>
#include <vcpu.h>
#include <smp.h>
#include <arch/arm/rtsm-config.h>

#include <stage2_mm.h>
#include <guest_mm.h>

#define L2_ENTRY_MASK 0x1FF
#define L2_SHIFT 21

#define L3_ENTRY_MASK 0x1FF
#define L3_SHIFT 12

extern vm_pgentry *_vmid_ttbl[NUM_GUESTS_STATIC];

#define VTTBR_INITVAL                                   0x0000000000000000ULL
#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFF000ULL
#define VTTBR_BADDR_SHIFT                               12

/** @} */

/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[31:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range,
 * VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */

/*
 * TODO: if you change the static variable, you will meet the system fault.
 * We don't konw about this issue, so we will checking this later time.
 */

/**
 * @brief Obtains TTBL_L3 entry.
 * Returns the address of TTBL l3 at 'index_l2' entry of L2.
 *
 * - vm_pgentry *TTBL_L3(vm_pgentry *ttbl_l2, uint32_t index_l2);
 *
 */

/**
 * @brief Configures Virtualization Translation Control Register(VTCR).
 *
 * Configure translation control bits by modifing the VTCR.
 * \ref VTCR
 * .
 * - Configuration.
 *   - SL0 - Start at first level
 *   - ORGN0 - Normal memory, outer write-back no write allocate cacheable.
 *   - IRGN0 - Normal memory, inner write-back no write allocate cacheable.
 *
 * @return void
 */

/**
 * @brief Enables or disables the stage-2 MMU.
 *
 * Configures Hyper Configuration Register(HCR) to enable or disable the
 * virtualization MMU.
 *
 * @param enable Enable or disable the MMU.
 *        - 1 : Enable the MMU.
 *        - 0 : Disable the MMU.
 * @return void
 */
static void guest_memory_stage2_enable(int enable)
{
    uint32_t hcr;
    /* HCR.VM[0] = enable */
    /* printf( "hcr:"); printf_hex32(hcr); printf("\n\r"); */
    hcr = read_hcr();
    if (enable)
        hcr |= (0x1);
    else
        hcr &= ~(0x1);

    write_hcr(hcr);
}

/**
 * @brief Changes the stage-2 translation table base address by configuring
 *        VTTBR.
 *
 * Configures Virtualization Translation Table Base Register(VTTBR) to change
 * the guest. Change vmid and base address from received vmid and ttbl
 * address.
 *
 * @param vmid Received vmid.
 * @param ttbl Level 1 translation table of the guest.
 * @return HVMM_STATUS_SUCCESS only.
 */
static hvmm_status_t guest_memory_set_vmid_ttbl(vmid_t vmid, vm_pgentry *ttbl)
{
    uint64_t vttbr;
    /*
     * VTTBR.VMID = vmid
     * VTTBR.BADDR = ttbl
     */
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    printf("current vttbr:");
    printf_hex64(vttbr);
    printf("\n\r");
#endif
    vttbr &= ~(VTTBR_VMID_MASK);
    vttbr |= ((uint64_t)vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
    vttbr &= ~(VTTBR_BADDR_MASK);
    vttbr |= (uint32_t) ttbl & VTTBR_BADDR_MASK;
    write_vttbr(vttbr);
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    printf("changed vttbr:");
    printf_hex64(vttbr);
    printf("\n\r");
#endif
    return HVMM_STATUS_SUCCESS;
}



/**
 * @brief Initializes the virtual mode(guest mode) memory management
 * stage-2 translation.
 *
 * Configure translation tables of guests for stage-2 translation (IPA -> PA).
 *
 * - First, maps physical address of guest start address to descriptors.
 * - And configure the translation table descriptors based on the memory
 *   map descriptor lists.
 * - Last, initializes mmu.
 *
 * @return void
 */

/**
 * @brief Configures the memory management features.
 *
 * Configure all features of the memory management.
 *
 * - Generate and confgirue hyp mode & virtual mode translation tables.
 * - Configure MAIRx, HMAIRx register.
 *   - \ref Memory_Attribute_Indirection_Register.
 *   - \ref Attribute_Indexes.
 * - Configures Hyp Translation Control Register(HTCR).
 *   - Setting
 *     - Out shareable.
 *     - Normal memory, outer write-through, cacheable.
 *     - Normal memory, inner write-back write-allocate cacheable.
 *     - T0SZ is zero.
 *   - \ref HTCR
 * - Configures Hyper System Control Register(HSCTLR).
 *   - i-cache and alignment checking enable.
 *   - mmu, d-cache, write-implies-xn, low-latency, IRQs disable.
 *   - \ref SCTLR
 * - Configures Hyp Translation Table Base Register(HTTBR).
 *   - Writes the vmm_pgtable value to base address bits.
 *   - \ref HTTBR
 * - Enable MMU and D-cache in HSCTLR.
 * - Initialize heap area.
 *
 * @return HVMM_STATUS_SUCCESS, Always success.
 */
static int memory_hw_init(struct memmap_desc **guest0,
            struct memmap_desc **guest1)
{
    uint32_t cpu = smp_processor_id();
    printf("[memory] memory_init: enter\n\r");

    stage2_mm_init(guest0, 0);
    stage2_mm_init(guest1, 1);

    guest_memory_init_mmu();

    printf("[memory] memory_init: exit\n\r");

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Stops stage-2 translation by disabling mmu.
 *
 * We assume VTCR has been configured and initialized in the memory
 * management module.
 * - Disables stage-2 translation by HCR.vm = 0.
 */
static hvmm_status_t memory_hw_save(void)
{
    /*
     * We assume VTCR has been configured and initialized
     * in the memory management module
     */
    /* Disable Stage 2 Translation: HCR.VM = 0 */
    guest_memory_stage2_enable(0);

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Restores translation table for the next guest and enable stage-2 mmu.
 *
 * - Chagne stage-2 translation table and vmid.
 * - Eanbles stage-2 MMU.
 *
 * @param guest Context of the next guest.
 */
static hvmm_status_t memory_hw_restore(vmid_t vmid)
{
    /*
     * Restore Translation Table for the next guest and
     * Enable Stage 2 Translation
     */
    guest_memory_set_vmid_ttbl(vmid, _vmid_ttbl[vmid]);

    guest_memory_stage2_enable(1);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t memory_hw_dump(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct memory_ops _memory_ops = {
    .init = memory_hw_init,
    .save = memory_hw_save,
    .restore = memory_hw_restore,
    .dump = memory_hw_dump,
};

struct memory_module _memory_module = {
    .name = "K-Hypervisor Memory Module",
    .author = "Kookmin Univ.",
    .ops = &_memory_ops,
};

