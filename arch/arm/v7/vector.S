    .syntax unified
    .arch_extension sec
    .arch_extension virt
    .text

    .align 5
    .global __monitor_vector
__monitor_vector:
    .word   0                       /* reset */
    nop                             /* und */
    b   smc_handler                 /* smc */
    nop                             /* pabt */
    nop                             /* dabt */
    b   hvc_handler                 /* hvc */
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

    @ reuse __monitor_vector to enter NSHyp mode temporarily
    ldr r11, = __monitor_vector
    mcr p15, 4, r11, c12, c0, 0
    @ return in NS state
    movs    pc, lr

hvc_handler:
    @ Clear bss section.
    ldr  r2, =__begin_bss
    ldr  r3, =__end_bss

    mov  r0, #0
1:  str  r0, [r2], #4    @ clear bss
    cmp  r2, r3
    blo  1b

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
    nop                         /* dabt */
    b    hvc_trap_handler       /* hvc */
    b    hvc_irq_handler        /* irq */
    nop                         /* fiq */

hvc_trap_handler:
    @ Push registers
    push    {r0-r12}
    mrs     r0, spsr_hyp
    mrs     r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_hvc_service(sp)
    mov    r0, sp
    bl    _hyp_hvc    @ r0: HSR

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

hvc_irq_handler:
    @ Push registers
    push    {r0-r12}
    mrs    r0, spsr_hyp
    mrs    r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hvc_irq_handler(sp)
    mov    r0, sp
    bl    _hyp_irq    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr        spsr_hyp, r0
    msr        elr_hyp, r1
    pop     {r0-r12}
    eret
