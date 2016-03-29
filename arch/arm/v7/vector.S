#include <asm/macro.h>
#include <config.h>

    .syntax unified
    .arch_extension sec
    .arch_extension virt

BEGIN_PROC(__monitor_vector)
    .word   0                       /* reset */
    nop                             /* und */
    b   smc_handler                 /* smc */
    nop                             /* pabt */
    nop                             /* dabt */
    b   hvc_handler                 /* hvc */
    nop                             /* irq */
    nop                             /* fiq */
END_PROC(__monitor_vector)

smc_handler:
    // Configure SCR
    mrc     p15, 0, r1, c1, c1, 0  @ SCR -> r10
    @ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
    bic     r1, r1, #0x07f
    ldr     r2, = 0x1b1
    orr     r1, r1, r2
    mcr     p15, 0, r2, c1, c1, 0
    isb

    @ reuse __monitor_vector to enter NSHyp mode temporarily
    ldr     r1, = __monitor_vector
    mcr     p15, 4, r1, c12, c0, 0
    movs    pc, lr  @ return in NS state

hvc_handler:
    ldr     sp, = __end_stack
    @ TODO(wonseok): make it smart.
    @ Adujst stack pointer for each core.
    mrc	    p15, 0, r5, c0, c0, 5		@ MPIDR (ARMv7 only)
    ands    r5, r5, #0xFF
    mov     r6, #HYP_STACK_SIZE
    mul     r6, r6, r5
    sub     sp, sp, r6
    mrs     lr, elr_hyp
    mov     pc, lr

    .align 5
BEGIN_PROC(__hvc_vector)
    .word   0                   /* reset */
    nop                         /* undef */
    nop                         /* svc */
    nop                         /* pabt */
    nop                         /* dabt */
    b       __do_hvc            /* hvc */
    b       __do_irq            /* irq */
    nop                         /* fiq */
END_PROC(__hvc_vector)

__do_hvc:
    @ Push registers
    push    {r0-r12}
    mrs     r0, spsr_hyp
    mrs     r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> _hyp_hvc_service(sp)
    mov    r0, sp
    bl     do_hvc_trap    @ r0: HSR

    @ r0 = return
    tst    r0, #1
    @ if return == HYP_RET_STAY -> stay in Hyp mode
    bne    1f

    @ Pop registers
    pop     {r0-r1, lr}
    msr     spsr_hyp, r0
    msr     elr_hyp, r1
    pop     {r0-r12}

    @ else if return == HYP_RET_ERET -> Exception Return
    eret

1:
    @ Pop registers
    pop     {r0-r1, lr}
    tst     r0, #0x1f
    msrne   spsr_hyp, r0
    msr     elr_hyp, r1
    pop     {r0-r12}
    @ stay in Hyp mode
    mrs     lr, elr_hyp
    mov     pc, lr

__do_irq:
    @ Push registers
    push    {r0-r12}
    mrs     r0, spsr_hyp
    mrs     r1, elr_hyp
    push    {r0, r1, lr}

    @ service other argument values -> trap_irq(sp)
    mov     r0, sp
    bl      do_irq    @ r0: HSR

    @ Pop registers
    pop     {r0-r1, lr}
    msr     spsr_hyp, r0
    msr     elr_hyp, r1
    pop     {r0-r12}
    eret
