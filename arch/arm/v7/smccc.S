#include <asm/macro.h>

    .syntax unified
    .arch_extension sec
    .arch_extension virt

BEGIN_PROC(arm_smccc_smc)
    smc #0
    bx  lr
END_PROC(arm_smccc_smc)

BEGIN_PROC(arm_smccc_hvc)
    hvc #0
    bx  lr
END_PROC(arm_smccc_hvc)
