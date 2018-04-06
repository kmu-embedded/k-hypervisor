#include <asm/macro.h>

    .syntax unified
    .arch_extension sec
    .arch_extension virt

BEGIN_PROC(arm_smccc_smc)
    mov     r12, sp
    push    {r4-r7}
    ldm     r12, {r4-r7}

    smc     #0

    pop     {r4-r7}
    ldr     r12, [sp, #(4 * 4)]
    stm     r12, {r0-r3}
    bx      lr
END_PROC(arm_smccc_smc)

BEGIN_PROC(arm_smccc_hvc)
    mov     r12, sp
    push    {r4-r7}
    ldm     r12, {r4-r7}

    hvc     #0

    pop     {r4-r7}
    ldr     r12, [sp, #(4 * 4)]
    stm     r12, {r0-r3}
    bx      lr
END_PROC(arm_smccc_hvc)
