#ifndef __LPAED_H__
#define __LPAED_H__

#include <stdint.h>

union lpae_t {
    uint64_t raw;

    struct {
        uint64_t valid: 1;
    } invalid;

    struct  {
        uint64_t valid: 1;
        uint64_t type: 1;
    } reserved;

    struct {
        /* bit[1:0] */
        uint64_t valid: 1;
        uint64_t type: 1;
        /* bit[11:2] */
        uint64_t mem_attr: 4;
        //uint64_t attr_indx:3;
        //uint64_t ns:1; // IGNORED
        uint64_t ap: 2; // ap[1] is SBO
        uint64_t sh: 2;
        uint64_t af: 1;
        uint64_t ng: 1; // SBZ in Non-Secure state.
        /* bit[51:12] */
        uint64_t base: 28;
        uint64_t sbzp: 12;
        /* bit[58:52] */
        uint64_t cb: 1;
        uint64_t pxn: 1; // SBZ in stage 1 translation.
        uint64_t xn: 1;
        uint64_t reserved: 4;

        /* bit[64:59] */
        uint64_t ignored: 5;
    } block __attribute__((__packed__));

    struct {
        /* bit[1:0] */
        uint64_t valid: 1;
        uint64_t type: 1;
        /* bit[11:2] */
        uint64_t lower_ignored: 10;
        /* bit[51:12] */
        uint64_t base: 28;
        uint64_t sbzp: 12;
        /* bit[58:52] */
        uint64_t upper_ignored: 7;
        /* bit[64:59] */
        uint64_t pxn_table: 1; // The pxn_table is SBZ in stage 1 translation.
        uint64_t xn_table: 1;
        uint64_t ap_table: 2; // The ap_table[0] is SBZ, in stage 1 translation.
        uint64_t ns_table: 1; // This bit is ignored in Non-Secure state.
    } table __attribute__((__packed__));

    /*
     * Actually, page and block has same attributes,
     * but we define the same structure again for readability
     */
    struct {
        /* bit[1:0] */
        uint64_t valid: 1;
        uint64_t type: 1;
        /* bit[11:2] */
        uint64_t mem_attr: 4;
        //uint64_t attr_indx:3;
        //uint64_t ns:1;
        uint64_t ap: 2;
        uint64_t sh: 2;
        uint64_t af: 1;
        uint64_t ng: 1;
        /* bit[51:12] */
        uint64_t base: 28;
        uint64_t sbzp: 12;
        /* bit[58:52] */
        uint64_t cb: 1;
        uint64_t pxn: 1;
        uint64_t xn: 1;
        uint64_t reserved: 4;
        /* bit[64:59] */
        uint64_t ignored: 5;
    } page __attribute__((__packed__));
};

typedef union lpae_t pgentry;

#define PAGE_SHIFT          12
#define ENTRY_SHIFT         9

#define L1_ENTRY            4
#define L2_ENTRY            512
#define L3_ENTRY            512

#define L3_SHIFT            PAGE_SHIFT
#define L2_SHIFT            (L3_SHIFT + ENTRY_SHIFT)
#define L1_SHIFT            (L2_SHIFT + ENTRY_SHIFT)

#define L1_BIT_MASK         0xC0000
#define L2_BIT_MASK         0x3FE00
#define L3_BIT_MASK         0x001FF

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

#define HMAIR0_VALUE 0x88440400
#define HMAIR1_VALUE 0x00FFCCBB
#define HTCR_VALUE	 0x80803500

typedef enum {
    MT_STRONGLY_ORDERED,    // 0
    MT_DEVICE,              // 1
    MT_NONCACHEABLE,        // 2
    MT_WRITETHROUGH_NO_ALLOC, // 3
    MT_WRITETHROUGH_RW_ALLOC,  // 4
    MT_WRITEBACK_NO_ALLOC, // 5
    MT_WRITEBACK_RW_ALLOC  // 6
} attr_indx_t;

typedef enum {
    NON_SHAREABLE,
    UNPREDICTABLE,
    OUTER_SHAREABLE,
    INNER_SHAREABLE
} shareability; // shareability

typedef enum {
    NONCACHEABLE,
    WRITEBACK_CACHEABLE,
    WRITETHROUGH_CACHEABLE,
    WRITEBACK_NONCACHEABLE
} cachebility; //cacheability;


typedef uint32_t addr_t;

pgentry set_table(addr_t pa);
void write_pgentry(addr_t base, addr_t va, addr_t pa, uint8_t mem_attr, uint8_t ap, uint8_t af);

#define set_valid      (1 << 0)
#define set_invalid    (0 << 0)

#define GET_OFFSET(i)        (i << 3)               /* size of pgentry */
#define GET_L2_INDEX(i)      GET_OFFSET((i << 9))   /* << 9 == 512 */
#define GET_L3_INDEX(i)      GET_OFFSET((i << 18))  /* << 18 == 512*512 */


#endif /* __LPAED_H__ */
