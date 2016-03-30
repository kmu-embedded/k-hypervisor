#ifndef __FSR_H__
#define __FSR_H__

#include <stdint.h>
#include <asm/asm.h>



/* Definition of Fault Status Registers */
#define read_hdfar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 0\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hdfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 0\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hifar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 2\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hifar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 2\n\t" \
                                : : "r" ((val)) : "memory", "cc")

#define read_hpfar()            ({ uint32_t rval; asm volatile(\
                                " mrc     p15, 4, %0, c6, c0, 4\n\t" \
                                : "=r" (rval) : : "memory", "cc"); rval; })

#define write_hpfar(val)        asm volatile(\
                                " mcr     p15, 4, %0, c6, c0, 4\n\t" \
                                : : "r" ((val)) : "memory", "cc")

/* Long-descriptor format FSR encodings */
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

#endif /* INCLUDE_ARCH_ARM_ARMV7_FSR_H_ */
