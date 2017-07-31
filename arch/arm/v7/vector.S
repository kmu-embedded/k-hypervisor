#include <asm/macro.h>
#include <config.h>
#include <arch/v7/cp15.h>

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
    write_cp32(r0, MVBAR)

    // Configure SCR
    @ SCR.NS=1, IRQ,FIQ,EA=0, FW,AW=1, nET=0, SCD=1, HCE=1
    read_cp32(r1, SCR)
    bic     r1, r1, #0x07f
    ldr     r2, = 0x1b1
    orr     r1, r1, r2
    write_cp32(r2, SCR)
    isb

    @ Reuse __monitor_vector to enter NSHyp mode temporarily.
    ldr     r1, = __monitor_vector
    write_cp32(r1, HVBAR)
    movs    pc, lr  @ return in NS state

hvc_handler:
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
    bl     do_hyp_trap    @ r0: HSR

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

