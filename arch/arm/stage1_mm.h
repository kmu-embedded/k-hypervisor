#ifndef __STAGE1_MM_H__
#define __STAGE1_MM_H__

union stage1_pgentry
{
    uint64_t raw;

    struct{
        uint64_t valid:1;
    } invalid;

    struct  {
        uint64_t valid:1;
        uint64_t type:1;
    } reserved;

    struct {
        /* bit[1:0] */
        uint64_t valid:1;
        uint64_t type:1;
        /* bit[11:2] */
        uint64_t attr_indx:3;
        uint64_t ns:1; // IGNORED
        uint64_t ap:2; // ap[1] is SBO
        uint64_t sh:2;
        uint64_t af:1;
        uint64_t ng:1; // SBZ in Non-Secure state.
        /* bit[51:12] */
        uint64_t base:28;
        uint64_t sbzp:12;
        /* bit[58:52] */
        uint64_t cb:1;
        uint64_t pxn:1; // SBZ in stage 1 translation.
        uint64_t xn:1;
        uint64_t reserved:4;

        /* bit[64:59] */
        uint64_t ignored:5;
    } block __attribute__((__packed__));

    struct {
        /* bit[1:0] */
        uint64_t valid:1;
        uint64_t type:1;
        /* bit[11:2] */
        uint64_t lower_ignored:10;
        /* bit[51:12] */
        uint64_t base:28;
        uint64_t sbzp:12;
        /* bit[58:52] */
        uint64_t upper_ignored:7;
        /* bit[64:59] */
        uint64_t pxn_table:1; // The pxn_table is SBZ in stage 1 translation.
        uint64_t xn_table:1;
        uint64_t ap_table:2; // The ap_table[0] is SBZ, in stage 1 translation.
        uint64_t ns_table:1; // This bit is ignored in Non-Secure state.
    } table __attribute__((__packed__));

    /*
     * Actually, page and block has same attributes,
     * but we define the same structure again for readability
     */
    struct {
        /* bit[1:0] */
        uint64_t valid:1;
        uint64_t type:1;
        /* bit[11:2] */
        uint64_t attr_indx:3;
        uint64_t ns:1;
        uint64_t ap:2;
        uint64_t sh:2;
        uint64_t af:1;
        uint64_t ng:1;
        /* bit[51:12] */
        uint64_t base:28;
        uint64_t sbzp:12;
        /* bit[58:52] */
        uint64_t cb:1;
        uint64_t pxn:1;
        uint64_t xn:1;
        uint64_t reserved:4;

        /* bit[64:59] */
        uint64_t ignored:5;
    } page __attribute__((__packed__));
};

typedef union stage1_pgentry pgentry;

/*
 * ARM memory attribute combinations
 *
 * Attrm[7:0]
 *
 * STRONG_ORDERED 0000_0000
 * DEVICE         0000_0100
 * NON_CACHEABLE  0100_0100
 * WRITETHROUGH_NO_ALLOC 1000_1000
 * WRITETHROUGH_RW_ALLOC 1011_1011
 * WRITEBACK_NO_ALLOC 1100_1100
 * WRITEBACK_RW_ALLOC 1111_1111
 *
 * - Used initial value of MAIR0, MAIR1.
 * <pre>
 * |     |31       24|23       16|15        8|7         0|
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR0| 1000 1000 | 0100 0100 | 0000 0100 | 0000 0000 |
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR1| 0000 0000 | 1111 1111 | 1100 1100 | 1011 1011 |
 */

#define MAIR0_VALUE 0x88440400
#define MAIR1_VALUE 0x00FFCCBB

hvmm_status_t stage1_pgtable_init();
hvmm_status_t stage1_mmu_init();

#endif /* __STAGE1_MM_H__ */
