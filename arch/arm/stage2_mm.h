#ifndef __LPAE_H__
#define __LPAE_H__

#include <vmem.h>

#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)

#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)

union stage2_pgentry
{
    uint64_t raw;

    struct{
        uint64_t valid:1; // == 0;
    } invalid;

    struct  {
        uint64_t valid:1; // == 1;
        uint64_t type:1; // == 0;
    } reserved;

    struct {
        /* bit[1:0] */
        uint64_t valid:1;
        uint64_t type:1;
        /* bit[11:2] */
        uint64_t mem_attr:4;
        uint64_t hap:2; // ap[1] is SBO
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
        uint64_t mem_attr:4;
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

typedef union stage2_pgentry vm_pgentry;

void stage2_mm_init(struct memmap_desc **guest_map, char ** _vmid_ttbl, vmid_t vmid);

#endif
