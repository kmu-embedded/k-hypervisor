#ifndef ____FSR_DECODE_H____
#define ____FSR_DECODE_H____

/* Long-descriptor format FSR decodings */
#define FSR_TRANS_FAULT(x)              (x + 0x04)
#define FSR_ACCESS_FAULT(x)             (x + 0x08)
#define FSR_PERM_FAULT(x)               (x + 0x0C)
#define FSR_SYNC_ABORT                  (0x10)
#define FSR_ASYNC_ABORT                 (0x11)
#define FSR_ABORT_ON_TABLE_WALK(x)      (x + 0x14)
#define FSR_SYNC_PERORR                 (0x18)
#define FSR_ASYNC_PERORR                (0x19)
#define FSR_PERORR_ON_TABLE_WALK(x)     (x + 0x1C)
#define FSR_ALINGMENT_FAULT             (0x21)
#define FSR_DEBUG_EVENT                 (0x22)
#define FSR_TLB_CONFLICT                (0x30)
#define FSR_DOMAIN_FAULT(x)             (x + 0x3C)

#endif /* __FSR_DECODE_H___ */
