#ifndef __LPAE_H__
#define __LPAE_H__

#include <vmem.h>

#define LPAE_PAGE_SHIFT    12
#define LPAE_PAGE_SIZE      (1<<LPAE_PAGE_SHIFT)
#define LPAE_PAGE_MASK      (0xFFF)

#define LPAE_BLOCK_L2_SHIFT 21
#define LPAE_BLOCK_L2_SIZE  (1<<LPAE_BLOCK_L2_SHIFT)
#define LPAE_BLOCK_L2_MASK  (0x1FFFFF)

void stage2_mm_init(struct memmap_desc **guest_map, char ** _vmid_ttbl, vmid_t vmid);

#endif
