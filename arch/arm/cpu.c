#include "cpu.h"
#include <arch/armv7.h>
#include <stdint.h>
#include <assert.h>

/* Set Hyp System Trap Register(HSTR) */
void enable_traps(void)
{
    uint32_t hstr = 0;

    hstr = 1 << 0;
    write_cp32(hstr, HSTR);
}
