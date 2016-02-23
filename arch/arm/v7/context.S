#include <asm/asm.h>

BEGIN_PROC(__set_vcpu_context_first_time)
    @ r0 = vcpu_regs->core_regs
    ldmfd    r0!, {r1, r2, r3}
    msr     spsr_hyp, r1    @ set spsr
    msr     elr_hyp, r2     @ set return address
    @msr     r2, r3			@ set return address
    ldm     r0, {r0-r12}    @ Currently, vcpu has no stack pointer.
                            @ so, restoring r0-r12 is enough.
    clrex
    eret
.type __set_vcpu_context_first_time, %function

