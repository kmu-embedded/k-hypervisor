#ifndef __MM_H__
#define __MM_H__

#define PGSIZE_4K 1
#if defined(PGSIZE_4K)

#define ENTRY_SHIFT 9
#define PAGE_SHIFT 12

#define L3_SHIFT    PAGE_SHIFT                  // 0x1000
#define L2_SHIFT    (L3_SHIFT + ENTRY_SHIFT)     // 0x100000
#define L1_SHIFT    (L2_SHIFT + ENTRY_SHIFT)

#define L1_ENTRY    4
#define L2_ENTRY    512
#define L3_ENTRY    512

#else // PAGE_2M
#endif // PAGE_2M

#endif /* __MM_H__*/
