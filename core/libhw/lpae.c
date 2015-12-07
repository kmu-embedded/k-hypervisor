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

/*
 * Level 1 Block, 1GB, entry in LPAE Descriptor format
 * for the given physical address
 */

union lpaed_t lpaed_host_l1_block(uint64_t pa, uint8_t attr_idx)
{
    /* lpae.c */
    union lpaed_t entry;
    printf("[mm] hvmm_mm_lpaed_l1_block:\n\r");
    printf(" pa: %llu\n", pa);
    printf(" attr_idx: 0x%08x\n", (uint32_t) attr_idx);

    /* Valid Block Entry */
    entry.stage1.valid = 1;
    entry.stage1.type = 0;
    entry.bits &= ~TTBL_L1_OUTADDR_MASK;
    entry.bits |= pa & TTBL_L1_OUTADDR_MASK;
    entry.stage1.sbzp = 0;

    /* Lower block attributes */
    entry.stage1.attr_indx = attr_idx;
    entry.stage1.ns = 1;

    entry.stage1.ap = 0;
    entry.stage1.sh = 3;
    entry.stage1.af = 1;
    entry.stage1.ng = 1;

    /* Upper block attributes */
    entry.stage1.cb = 0;
    entry.stage1.pxn = 0;
    entry.stage1.xn = 0;
    return entry;
}

/* Level 1 Table, 1GB, each entry refer level2 page table */
union lpaed_t lpaed_host_l1_table(uint64_t pa)
{
    union lpaed_t entry;
    /* Valid Table Entry */
    entry.stage1.valid = 1;
    entry.stage1.type = 1;
    /* Next-level table address [39:12] */
    entry.bits &= ~TTBL_L1_TABADDR_MASK;
    entry.bits |= pa & TTBL_L1_TABADDR_MASK;
    /* UNK/SBZP [51:40] */
    entry.stage1.sbzp = 0;
    entry.stage1.pxnt = 0;  /* PXN limit for subsequent levels of lookup */
    entry.stage1.xnt = 0;   /*  XN limit for subsequent levels of lookup */
    /*  Access permissions limit for subsequent levels of lookup */
    entry.stage1.apt = 0;
    /*  Table address is in the Non-secure physical address space */
    entry.stage1.nst = 1;
    return entry;
}

/* Level 2 Table, 2MB, each entry refer level3 page table.*/
union lpaed_t lpaed_host_l2_table(uint64_t pa)
{
    union lpaed_t entry;
    /* Valid Table Entry */
    entry.stage1.valid = 1;
    entry.stage1.type = 1;

    /* Next-level table address [39:12] */
    entry.bits &= ~TTBL_L2_TABADDR_MASK;
    entry.bits |= pa & TTBL_L2_TABADDR_MASK;

    /* UNK/SBZP [51:40] */
    entry.stage1.sbzp = 0;
    entry.stage1.pxnt = 0;  /* PXN limit for subsequent levels of lookup */
    entry.stage1.xnt = 0;   /* XN limit for subsequent levels of lookup */

    /* Access permissions limit for subsequent levels of lookup */
    entry.stage1.apt = 0;
    /* Table address is in the Non-secure physical address space */
    entry.stage1.nst = 1;
    return entry;
}

/* Level 3 Table, each entry refer 4KB physical address */
union lpaed_t lpaed_host_l3_table(uint64_t pa,
        uint8_t attr_idx, uint8_t valid)
{
    union lpaed_t entry;
    /*  Valid Table Entry */
    entry.stage1.valid = valid;
    entry.stage1.type = 1;
    /*  4KB physical address [39:12] */
    entry.bits &= ~TTBL_L3_OUTADDR_MASK;
    entry.bits |= pa & TTBL_L3_OUTADDR_MASK;
    /*  UNK/SBZP [51:40] */
    entry.stage1.sbzp = 0;
    /* Lower page attributes */
    entry.stage1.attr_indx = attr_idx;
    entry.stage1.ns = 1;    /*  Allow Non-secure access */
    entry.stage1.ap = 1;    /*  Outher Shareable */
    entry.stage1.sh = 3;    /*  Outher Shareable */
    entry.stage1.af = 1;    /*  Access Flag set to 1? */
    entry.stage1.ng = 1;
    /*  Upper page attributes */
    entry.stage1.cb = 0;
    entry.stage1.pxn = 0;
    entry.stage1.xn = 0;    /*  eXecute Never = 0 */
    return entry;
}

void lpaed_guest_stage2_conf_l1_table(union lpaed_t *ttbl1,
        uint64_t baddr, uint8_t valid)
{
    ttbl1->stage2.valid = valid ? 1 : 0;
    ttbl1->stage2.type = valid ? 1 : 0;
    ttbl1->bits &= ~TTBL_L1_TABADDR_MASK;
    ttbl1->bits |= baddr & TTBL_L1_TABADDR_MASK;
}

void lpaed_guest_stage2_conf_l2_table(union lpaed_t *ttbl2,
        uint64_t baddr, uint8_t valid)
{
    ttbl2->stage2.valid = valid ? 1 : 0;
    ttbl2->stage2.type = valid ? 1 : 0;
    ttbl2->bits &= ~TTBL_L2_TABADDR_MASK;
    ttbl2->bits |= baddr & TTBL_L2_TABADDR_MASK;
}

void lpaed_guest_stage2_enable_l2_table(union lpaed_t *ttbl2)
{
    ttbl2->stage2.valid = 1;
    ttbl2->stage2.type = 1;
}
void lpaed_guest_stage2_disable_l2_table(union lpaed_t *ttbl2)
{
    ttbl2->stage2.valid = 0;
}

void lpaed_guest_stage2_map_page(union lpaed_t *pte, uint64_t pa,
        enum memattr mattr)
{
    pte->stage2.valid = 1;
    pte->stage2.type = 1;
    pte->bits &= ~TTBL_L3_OUTADDR_MASK;
    pte->bits |= pa & TTBL_L3_OUTADDR_MASK;
    pte->stage2.sbzp = 0;
    /* Lower block attributes */
    pte->stage2.mattr = mattr & 0x0F;
    pte->stage2.ap = 3;    /* Non-shareable */
    pte->stage2.sh = 0;    /* Non-shareable */
    pte->stage2.af = 1;    /* Access Flag set to 1? */
    pte->stage2.ng = 0;
    /* Upper block attributes */
    pte->stage2.cb = 0;
    pte->stage2.pxn = 0;
    pte->stage2.xn = 0;    /* eXecute Never = 0 */
    pte->stage2.reserved = 0;
}

void lpaed_guest_stage1_conf_l3_table(union lpaed_t *ttbl3,
        uint64_t baddr, uint8_t valid)
{
    ttbl3->stage1.valid = valid ? 1 : 0;
    ttbl3->bits &= ~TTBL_L3_OUTADDR_MASK;
    ttbl3->bits |= baddr & TTBL_L3_OUTADDR_MASK;
}

void lpaed_guest_stage1_disable_l3_table(union lpaed_t *ttbl3)
{
    ttbl3->stage1.valid = 0;
}
