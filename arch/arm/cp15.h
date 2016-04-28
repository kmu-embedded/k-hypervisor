#ifndef __CP_15_H__
#define __CP_15_H__

#include <arch/armv7.h>

int32_t emulate_cp15(struct core_regs *regs, uint32_t iss);

#endif // __CP15_H__
