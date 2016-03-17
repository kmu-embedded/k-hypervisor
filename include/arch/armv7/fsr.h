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

#endif /* INCLUDE_ARCH_ARM_ARMV7_FSR_H_ */
