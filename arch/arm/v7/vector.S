#include <arch/arm/rtsm-config.h>

    .syntax unified
    .arch_extension sec
    .arch_extension virt
    .text

    .align 5
    .global __mon_vector
__mon_vector:
    .word   0                       /* reset */
    nop                             /* und */
    b   smc_handler                 /* smc */
    nop                             /* pabt */
    nop                             /* dabt */
    b   one_time_hvc_handler        /* hvc */
    nop                             /* irq */
    nop                             /* fiq */

smc_handler:
    // Configure SCR
    mrc p15, 0, r10, c1, c1, 0  @ SCR -> r10
    @ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
    bic r10, r10, #0x07f
    ldr r11, = 0x1b1
    orr r10, r10, r11
    mcr p15, 0, r11, c1, c1, 0
    isb

    @ reuse __mon_vector to enter NSHyp mode temporarily
    ldr r11, = __mon_vector
    mcr p15, 4, r11, c12, c0, 0
    @ return in NS state
    movs    pc, lr

one_time_hvc_handler:
    /* Setup stack for Hyp for the first time */
    ldr sp, = __end_stack
    mrs lr, elr_hyp
    mov pc, lr

    .align 5
    .global __hvc_vector
__hvc_vector:
    .word   0                   /* reset */
    nop                         /* undef */
    nop                         /* svc */
    nop                         /* pabt */
    b    hvc_dabt_handler       /* dabt */
    b    hvc_trap_handler       /* hvc */
    b    hvc_irq_handler        /* irq */
    nop                         /* fiq */

hvc_trap_handler:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_hvc_service(sp)
    mov    r0, sp
    @bl    _hyp_hvc    @ r0: HSR

    @ r0 = return
    tst    r0, #1
    @ if return == HYP_RET_STAY -> stay in Hyp mode
    bne    1f

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}

    @ else if return == HYP_RET_ERET -> Exception Return
    eret

1:
    @ Pop registers
    pop     {r0-r1, lr}
    tst     r0, #0x1f
    msrne    spsr_hyp, r0
    msr    elr_hyp, r1
    pop     {r0-r12}
    @ stay in Hyp mode
    mrs    lr, elr_hyp
    mov    pc, lr

hvc_dabt_handler:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_trap_dabort(sp)
    mov    r0, sp
    @bl    _hyp_dabort    @ r0: HSR

    @ if (return == 0) execute a conditional code
    @ tst    r0, #1
    @ <conditional code>

    @ Pop registers
    pop     {r0-r1, lr}
    msr    spsr_hyp, r0
    msr    elr_hyp, r1
    pop     {r0-r12}
    eret

hvc_irq_handler:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hvc_irq_handler(sp)
    mov    r0, sp
    @bl    _hyp_irq    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}
    eret
