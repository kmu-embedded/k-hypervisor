    .global __mon_switch_to_guest_context
__mon_switch_to_guest_context:    @ struct arch_regs *r0
    ldmfd    r0!, {r1, r3, lr}
    msr     spsr_hyp, r1
    msr     elr_hyp, r3
    ldm     r0, {r0-r12}
    clrex
    eret
.type __mon_switch_to_guest_context, %function

