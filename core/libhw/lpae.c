#include <stdio.h>
#include <lpae.h>

/**
 * \defgroup LPAE_address_mask
 *
 * The address mask of each level descriptors.
 * - TTBL_L1_OUTADDR_MASK[39:30] Level1 Block address mask.
 * - TTBL_L2_OUTADDR_MASK[39:21] Level2 Block Address mask.
 * - TTBL_L3_OUTADDR_MASK[39:12] Page address mask.
 *
 * - TTBL_L1_TABADDR_MASK[39:12] Level2 table descriptor address mask.
 * - TTBL_L2_TABADDR_MASK]30:12] Level3 table descriptor address mask.
 * @{
 */
#define TTBL_L1_OUTADDR_MASK    0x000000FFC0000000ULL
#define TTBL_L2_OUTADDR_MASK    0x000000FFFFE00000ULL
#define TTBL_L3_OUTADDR_MASK    0x000000FFFFFFF000ULL

#define TTBL_L1_TABADDR_MASK    0x000000FFFFFFF000ULL
#define TTBL_L2_TABADDR_MASK    0x000000FFFFFFF000ULL

#define is_table()      0x1
#define is_page()       0x1
#define is_level1()     1
#define is_level2()     2
#define is_level3()     3

#define PAGE_MASK           (~(4096-1))
#define PADDR_BITS              40
#define PADDR_MASK              ((1ULL << PADDR_BITS)-1)

lpaed_t create_hypervisor_pagetable(uint32_t paddr, uint8_t desc_type, uint8_t level, uint8_t attr_idx)
{
    lpaed_t entry;
    entry.bits = 0x00000000ULL;

    entry.s1_page.valid = 1;
    entry.s1_page.type = desc_type;
    //printf("\t\tpaddr: 0x%08x\n", paddr);
    //printf("\t\t: entry.bits: 0x%08x\n", entry.bits);

    if(desc_type == 3) {
        entry.s1_page.type = 1;
    }

    if(desc_type != is_table()) { // page or block
        /* Lower block attributes */
        entry.s1_page.attr_indx = attr_idx;
        entry.s1_page.ns = 0;
        entry.s1_page.ap = 0;
        entry.s1_page.sh = 3;
        entry.s1_page.af = 1;
        entry.s1_page.ng = 1;

        //entry.s1_page.output |= paddr;
        entry.bits |= paddr;
#if 0
        if(level == is_level1()) {
            entry.bits &= ~TTBL_L1_OUTADDR_MASK;
            entry.bits |= paddr & TTBL_L1_OUTADDR_MASK;
        } else if (level == is_level2()) {
            printf("==========\n");
            printf("\t\tlv2: paddr: 0x%08x\n", paddr);
            entry.s1_page.output |= paddr;
            printf("\t\tlv2: entry.bits: 0x%08x\n", entry.bits);
            printf("==========\n");
        } else if (level == is_level3()) {
            //printf("==========\n");
            //printf("\t\tlv3: paddr: 0x%08x\n", paddr);
            entry.bits |= paddr;
            //entry.bits &= ~TTBL_L3_OUTADDR_MASK;
            //entry.bits |= paddr & TTBL_L3_OUTADDR_MASK;
            //printf("\t\tlv3: entry.bits: 0x%08x\n", entry.bits);
            //printf("==========\n");
        }
#endif
        entry.s1_page.sbzp = 0;

        entry.s1_page.cb = 0;
        entry.s1_page.pxn = 0;
        entry.s1_page.xn = 0;
    } else { // table

#if 0
        if(level == is_level1()) {
            entry.bits &= ~TTBL_L1_TABADDR_MASK;
            entry.bits |= paddr & TTBL_L1_TABADDR_MASK;
            entry.s1_page.sbzp = 0;
        } else {
            entry.bits &= ~TTBL_L2_TABADDR_MASK;
            entry.bits |= paddr & TTBL_L2_TABADDR_MASK;
            entry.s1_page.sbzp = 0;
        }
#endif
        entry.bits |= paddr;
        entry.s1_page.sbzp = 0;

        entry.s1_page.pxn_table = 0;
        entry.s1_page.xn_table = 0;
        entry.s1_page.ap_table = 0;
        entry.s1_page.ns_table = 1;
    }
    //printf("\t\t0x%08x\n", entry.bits);


    return entry;
}


lpaed_t lpaed_host_l1_block(uint64_t pa, uint8_t attr_idx)
{
    /* lpae.c */
    lpaed_t entry;

    entry.s1_page.valid = 1;
    entry.s1_page.type = 0;

    /* Lower block attributes */
    entry.s1_page.attr_indx = attr_idx;
    entry.s1_page.ns = 0;
    entry.s1_page.ap = 0;
    entry.s1_page.sh = 3;
    entry.s1_page.af = 1;
    entry.s1_page.ng = 1;

    entry.bits &= ~TTBL_L1_OUTADDR_MASK;
    entry.bits |= pa & TTBL_L1_OUTADDR_MASK;
    entry.s1_page.sbzp = 0;

    entry.s1_page.cb = 0;
    entry.s1_page.pxn = 0;
    entry.s1_page.xn = 0;
    return entry;
}

lpaed_t lpaed_host_l1_table(uint64_t pa)
{
    lpaed_t entry;

    entry.s1_page.valid = 1;
    entry.s1_page.type = 1;

    entry.bits &= ~TTBL_L1_TABADDR_MASK;
    entry.bits |= pa & TTBL_L1_TABADDR_MASK;
    entry.s1_page.sbzp = 0;

    entry.s1_page.pxn_table = 0;
    entry.s1_page.xn_table = 0;
    entry.s1_page.ap_table = 0;
    entry.s1_page.ns_table = 1;
    return entry;
}

lpaed_t lpaed_host_l2_table(uint64_t pa)
{
    lpaed_t entry;

    entry.s1_page.valid = 1;
    entry.s1_page.type = 1;

    entry.bits &= ~TTBL_L2_TABADDR_MASK;
    entry.bits |= pa & TTBL_L2_TABADDR_MASK;
    entry.s1_page.sbzp = 0;

    entry.s1_page.pxn_table = 0;
    entry.s1_page.xn_table = 0;
    entry.s1_page.ap_table = 0;
    entry.s1_page.ns_table = 1;
    return entry;
}

/* Level 3 Table, each entry refer 4KB physical address */
lpaed_t lpaed_host_l3_table(uint64_t pa,
        uint8_t attr_idx, uint8_t valid)
{
    lpaed_t entry;

    entry.s1_page.valid = valid;
    entry.s1_page.type = 1;

    entry.s1_page.attr_indx = attr_idx;
    entry.s1_page.ns = 0;
    entry.s1_page.ap = 0;
    entry.s1_page.sh = 3;
    entry.s1_page.af = 1;
    entry.s1_page.ng = 1;

    /*  4KB physical address [39:12] */
    entry.bits &= ~TTBL_L3_OUTADDR_MASK;
    entry.bits |= pa & TTBL_L3_OUTADDR_MASK;
    entry.s1_page.sbzp = 0;

    entry.s1_page.cb = 0;
    entry.s1_page.pxn = 0;
    entry.s1_page.xn = 0;
    return entry;
}

void lpaed_guest_stage2_conf_l1_table(lpaed_t *ttbl1,
        uint64_t baddr, uint8_t valid)
{
    ttbl1->s2_page.valid = valid ? 1 : 0;
    ttbl1->s2_page.type = valid ? 1 : 0;
    ttbl1->bits &= ~TTBL_L1_TABADDR_MASK;
    ttbl1->bits |= baddr & TTBL_L1_TABADDR_MASK;
}

void lpaed_guest_stage2_conf_l2_table(lpaed_t *ttbl2,
        uint64_t baddr, uint8_t valid)
{
    ttbl2->s2_page.valid = valid ? 1 : 0;
    ttbl2->s2_page.type = valid ? 1 : 0;
    ttbl2->bits &= ~TTBL_L2_TABADDR_MASK;
    ttbl2->bits |= baddr & TTBL_L2_TABADDR_MASK;
}

void lpaed_guest_stage2_enable_l2_table(lpaed_t *ttbl2)
{
    ttbl2->s2_page.valid = 1;
    ttbl2->s2_page.type = 1;
}
void lpaed_guest_stage2_disable_l2_table(lpaed_t *ttbl2)
{
    ttbl2->s2_page.valid = 0;
}

void lpaed_guest_stage2_map_page(lpaed_t *pte, uint64_t pa,
        enum memattr mattr)
{
    pte->s2_page.valid = 1;
    pte->s2_page.type = 1;

    /* Lower block attributes */
    pte->s2_page.mattr = mattr & 0x0F;
    pte->s2_page.hap = 3;
    pte->s2_page.sh = 0;
    pte->s2_page.af = 1;
    pte->s2_page.ng = 0;

    pte->bits &= ~TTBL_L3_OUTADDR_MASK;
    pte->bits |= pa & TTBL_L3_OUTADDR_MASK;
    pte->s2_page.sbzp = 0;

    /* Upper block attributes */
    pte->s2_page.cb = 0;
    pte->s2_page.pxn = 0;
    pte->s2_page.xn = 0;
    pte->s2_page.ignored = 0;
}

void lpaed_guest_stage1_conf_l3_table(lpaed_t *ttbl3,
        uint64_t baddr, uint8_t valid)
{
    ttbl3->s1_page.valid = valid ? 1 : 0;
    ttbl3->bits &= ~TTBL_L3_OUTADDR_MASK;
    ttbl3->bits |= baddr & TTBL_L3_OUTADDR_MASK;
}

void lpaed_guest_stage1_disable_l3_table(lpaed_t *ttbl3)
{
    ttbl3->s1_page.valid = 0;
}
