#ifndef __SMCCC_H__
#define __SMCCC_H__

#include <arch/armv7.h>

void arm_smccc_smc(unsigned long arg0, unsigned long arg1,
        unsigned long arg2, unsigned long arg3);

void arm_smccc_hvc(unsigned long arg0, unsigned long arg1,
        unsigned long arg2, unsigned long arg3);

int emulate_arm_smccc(struct core_regs *regs);

#endif
