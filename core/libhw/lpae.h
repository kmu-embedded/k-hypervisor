#ifndef __LPAE_H__
#define __LPAE_H__

#include <stdint.h>
#include <memory.h>

#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)

#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)

/**
 * \defgroup Attribute_Indexes
 *
 * These are valid in the AttrIndx[2:0] field of an LPAE stage-1 page
 * table entry. They are indexes into the bytes of the MAIRx registers.
 * The 8-bit fields are packed little-endian into MAIR0 and MAIR1.
 * /ref Memory_Attribute_Indirection_Register "MAIRx"
 * <pre>
 *                          ai    encoding
 *   ATTR_IDX_UNCACHED      000   0000 0000  -- Strongly Ordered
 *   ATTR_IDX_BUFFERABLE    001   0100 0100  -- Non-Cacheable
 *   ATTR_IDX_WRITETHROUGH  010   1010 1010  -- Write-through
 *   ATTR_IDX_WRITEBACK     011   1110 1110  -- Write-back
 *   ATTR_IDX_DEV_SHARED    100   0000 0100  -- Device
 *            ??            101
 *            reserved      110
 *   ATTR_IDX_WRITEALLOC    111   1111 1111  -- Write-back write-allocate
 *
 *   ATTR_IDX_DEV_NONSHARED 100   (== ATTR_IDX_DEV_SHARED)
 *   ATTR_IDX_DEV_WC        001   (== ATTR_IDX_BUFFERABLE)
 *   ATTR_IDX_DEV_CACHED    011   (== ATTR_IDX_WRITEBACK)
 *   </pre>
 * @{
 */
#define ATTR_IDX_UNCACHED      0x0
#define ATTR_IDX_BUFFERABLE    0x1
#define ATTR_IDX_WRITETHROUGH  0x2
#define ATTR_IDX_WRITEBACK     0x3
#define ATTR_IDX_DEV_SHARED    0x4
#define ATTR_IDX_WRITEALLOC    0x7
#define ATTR_IDX_DEV_NONSHARED ATTR_IDX_DEV_SHARED
#define ATTR_IDX_DEV_WC        ATTR_IDX_BUFFERABLE
#define ATTR_IDX_DEV_CACHED    ATTR_IDX_WRITEBACK

struct invalid
{
    unsigned long valid:1;
};

struct reserved
{
    unsigned long valid:1;
};

struct stage1_entry
{
    // bit[1:0]
    unsigned long valid:1;
    unsigned long type:1;

    // bit[11:2]
    unsigned long attr_indx:3;
    unsigned long ns:1;
    unsigned long ap:2;
    unsigned long sh:2;
    unsigned long af:1;
    unsigned long ng:1; // SBZ in stage 2 address translation.

    // bit[51:12]
    unsigned long output:28;
    unsigned long sbzp:12;

    // bit[63:52]
    unsigned long cb:1;
    unsigned long pxn:1; // SBZ in stage 2 address translation.
    unsigned long xn:1;
    unsigned long reserved:4;
    unsigned long pxn_table:1;
    unsigned long xn_table:1;
    unsigned long ap_table:2;
    unsigned long ns_table:1;
} __attribute__((__packed__));

struct stage2_entry {
    unsigned long valid:1;
    unsigned long type:1;

    unsigned long mattr:4;
    unsigned long hap:2;
    //unsigned long read:1;
    //unsigned long write:1;
    unsigned long sh:2;
    unsigned long af:1;
    unsigned long ng:1; // SBZ

    unsigned long base:28;
    unsigned long sbzp:12;

    unsigned long cb:1;
    unsigned long pxn:1;    // SBZ
    unsigned long xn:1;
    unsigned long reserved:4;
    unsigned long ignored:5;
} __attribute__((__packed__));


/**
 * @brief Walk entry.
 *
 * Walk is the common bits of s2 and pt entries which are needed to
 * simply walk the table. (e.g. for debug).
 *
 * - \ref Long_descriptor "Details"
 *
 * - Description
 *   - valid - Valid mapping
 *   - table - == 1 in 4k map entries too
 *   - pad2 -
 *   - base - Base address of block or next table
 *   - pad1 -
 */
struct lpae_walk {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;     /**< Valid mapping. */
    unsigned long table:1;     /**< == 1 in 4k map entries too. */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long base:28;     /**< Base address of block or next table. */

    unsigned long pad1:24;
} __attribute__((__packed__));

union lpaed_t {
    uint64_t bits;
    struct stage1_entry s1_page;
    struct stage2_entry s2_page;
    struct lpae_walk walk;
};
typedef union lpaed_t lpaed_t;

lpaed_t lpaed_host_l1_block(uint64_t pa, uint8_t attr_idx);
lpaed_t lpaed_host_l1_table(uint64_t pa);
lpaed_t lpaed_host_l2_table(uint64_t pa);

lpaed_t lpaed_host_l3_table(uint64_t pa, uint8_t attr_idx,
                uint8_t valid);
void lpaed_guest_stage1_conf_l3_table(lpaed_t *ttbl3, uint64_t baddr,
                uint8_t valid);
void lpaed_guest_stage1_disable_l3_table(lpaed_t *ttbl2);

void lpaed_guest_stage2_map_page(lpaed_t *pte, uint64_t pa,
        enum memattr mattr);
void lpaed_guest_stage2_conf_l1_table(lpaed_t *ttbl1, uint64_t baddr,
        uint8_t valid);
void lpaed_guest_stage2_conf_l2_table(lpaed_t *ttbl2, uint64_t baddr,
        uint8_t valid);
void lpaed_guest_stage2_enable_l2_table(lpaed_t *ttbl2);
void lpaed_guest_stage2_disable_l2_table(lpaed_t *ttbl2);

lpaed_t create_hypervisor_pagetable(uint32_t paddr, uint8_t pg_type, uint8_t level, uint8_t attr_idx);
#endif

