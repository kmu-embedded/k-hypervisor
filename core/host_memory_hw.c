#include <stdio.h>
#include <armv7_p15.h>
#include <stdint.h>
#include <hvmm_trace.h>
#include <lpae.h>
#include <memory.h>
#include <vcpu.h>
#include <smp.h>
#include <arch/arm/rtsm-config.h>

#include <host_memory_hw.h>

/* PL2 Stage 1 Level 1 */
#define HMM_L1_PTE_NUM  4

/* PL2 Stage 1 Level 2 */
#define HMM_L2_PTE_NUM  512

/* PL2 Stage 1 Level 3 */
#define HMM_L3_PTE_NUM  512

static union lpaed_t vmm_pgtable[HMM_L1_PTE_NUM] __attribute((__aligned__(4096)));
static union lpaed_t vmm_pgtable_l2[HMM_L2_PTE_NUM]  __attribute((__aligned__(4096)));
static union lpaed_t vmm_pgtable_l3[HMM_L2_PTE_NUM][HMM_L3_PTE_NUM]  __attribute((__aligned__(4096)));

void host_memory_init(void)
{
    int i, j;
    uint64_t pa = 0x00000000ULL;

    vmm_pgtable[0] = lpaed_host_l1_block(pa, ATTR_IDX_DEV_SHARED);
    pa = 0x40000000;

    // TODO(wonseok): Do we need to add a memory region?: 0x4000_0000 ~ 0x8000_0000
    vmm_pgtable[1] = lpaed_host_l1_block(pa, ATTR_IDX_UNCACHED);
    pa = 0x80000000;
    vmm_pgtable[2] = lpaed_host_l1_block(pa, ATTR_IDX_UNCACHED);

    pa = 0xC0000000;
    vmm_pgtable[3] = lpaed_host_l1_table((uint32_t) &vmm_pgtable_l2);

    for (i = 0; i < HMM_L2_PTE_NUM; i++) {

        vmm_pgtable_l2[i] =
                lpaed_host_l2_table((uint32_t) vmm_pgtable_l3[i]);

        for (j = 0; j < HMM_L3_PTE_NUM; pa += 0x1000, j++) {
                vmm_pgtable_l3[i][j] =
                        lpaed_host_l3_table(pa, ATTR_IDX_WRITEALLOC, 1);
        }
    }
}

int memory_enable(void)
{
/*
 *    MAIR0, MAIR1
 *    HMAIR0, HMAIR1
 *    HTCR
 *    HTCTLR
 *    HTTBR
 *     HTCTLR
 */
    uint32_t mair, htcr, hsctlr, hcr;
    uint64_t httbr;

    /* MAIR/HMAIR */
    printf(" --- MAIR ----\n\r");
    mair = read_mair0();
    printf("mair0: 0x%08x\n", mair);
    mair = read_mair1();
    printf("mair1: 0x%08x\n", mair);
    mair = read_hmair0();
    printf("hmair0: 0x%08x\n", mair);
    mair = read_hmair1();
    printf("hmair1: 0x%08x\n", mair);

    write_mair0(INITIAL_MAIR0VAL);
    write_mair1(INITIAL_MAIR1VAL);
    write_hmair0(INITIAL_MAIR0VAL);
    write_hmair1(INITIAL_MAIR1VAL);

    mair = read_mair0();
    mair = read_mair0();
    printf("mair0: 0x%08x\n", mair);
    mair = read_mair1();
    printf("mair1: 0x%08x\n", mair);
    mair = read_hmair0();
    printf("hmair0: 0x%08x\n", mair);
    mair = read_hmair1();
    printf("hmair1: 0x%08x\n", mair);

    /* HTCR */
    printf(" --- HTCR ----\n\r");
    htcr = read_htcr();
    printf("htcr: 0x%08x\n", htcr);
    write_htcr(0x80002500);
    htcr = read_htcr();
    printf("htcr: 0x%08x\n", htcr);

    /* HSCTLR */
    /* i-Cache and Alignment Checking Enabled */
    /* MMU, D-cache, Write-implies-XN, Low-latency IRQs Disabled */
    hsctlr = read_hsctlr();
    printf("hsctlr: 0x%08x\n", hsctlr);
    hsctlr = HSCTLR_BASE | SCTLR_A;
    write_hsctlr(hsctlr);
    hsctlr = read_hsctlr();
    printf("hsctlr: 0x%08x\n", hsctlr);

    /* HCR */
    hcr = read_hcr();
    printf("hcr: 0x%08x\n", hcr);

    /* HTCR */
    /*
     * Shareability - SH0[13:12] = 0 - Not shared
     * Outer Cacheability - ORGN0[11:10] = 11b -
     *                          Write Back no Write Allocate Cacheable
     * Inner Cacheability - IRGN0[9:8] = 11b - Same
     * T0SZ[2:0] = 0 - 2^32 Input Address
     */
    /* Untested code commented */
/*
    htcr = read_htcr();
    printf("htcr:");
    printf_hex32(htcr);
    printf("\n\r");
    htcr &= ~HTCR_SH0_MASK;
    htcr |= (0x0 << HTCR_SH0_SHIFT) & HTCR_SH0_MASK;
    htcr &= ~HTCR_ORGN0_MASK;
    htcr |= (0x3 << HTCR_ORGN0_SHIFT) & HTCR_ORGN0_MASK;
    htcr &= ~VTCR_IRGN0_MASK;
    htcr |= (0x3 << HTCR_IRGN0_SHIFT) & HTCR_IRGN0_MASK;
    htcr &= ~VTCR_T0SZ_MASK;
    htcr |= (0x0 << HTCR_T0SZ_SHIFT) & HTCR_T0SZ_MASK;
    write_htcr(htcr);
    htcr = read_htcr();
    printf("htcr:");
    printf_hex32(htcr);
    printf("\n\r");
*/

    /* HTTBR = &_vmm_pgtable */
    httbr = read_httbr();
    printf("httbr: %llu\n", httbr);
    httbr &= 0xFFFFFFFF00000000ULL;
    httbr |= (uint32_t) &vmm_pgtable;
    httbr &= HTTBR_BADDR_MASK;
    printf("writing httbr: %llu\n", httbr);
    write_httbr(httbr);
    httbr = read_httbr();
    printf("read back httbr: %llu\n", httbr);

    /* Enable PL2 Stage 1 MMU */

    hsctlr = read_hsctlr();
    printf("hsctlr: 0x%08x\n", hsctlr);

    /* HSCTLR Enable MMU and D-cache */
    hsctlr |= (SCTLR_M | SCTLR_C);

    /* Flush PTE writes */
    asm("dsb");

    write_hsctlr(hsctlr);

    /* Flush iCache */
    asm("isb");

    hsctlr = read_hsctlr();
    printf("hsctlr: 0x%08x\n", hsctlr);

    return HVMM_STATUS_SUCCESS;
}
