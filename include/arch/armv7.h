#ifndef __ARMV7_H__
#define __ARMV7_H__

#include <asm/asm.h>

#include "armv7/cache.h"
#include "armv7/fsr.h"
#include "armv7/tlb.h"
#include "armv7/sysctrl.h"
#include "armv7/vmsa.h"
#include "armv7/generic_timer.h"
#include "armv7/arm_inline.h"
#include "armv7/smp.h"
#include "armv7/mutex.h"

// FIXME: Move get_periphbase() to proper location.
static inline uint64_t get_periphbase(void)
{
    uint64_t periphbase = 0UL;
    unsigned long cbar = read_cbar();
    uint64_t upper_periphbase = cbar & 0xFF;

    if (upper_periphbase != 0x0) {
        periphbase |= upper_periphbase << 32;
        cbar &= ~(0xFF);
    }
    periphbase |= cbar;

    return periphbase;
}

#endif /* armv7.h */
