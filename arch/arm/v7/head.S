#include <asm/macro.h>
#include <config.h>

    .syntax	unified
    .arch_extension sec
    .arch_extension virt

BEGIN_PROC(__start)
    @ Disable interrupts
    cpsid   aif

    @ Check whether or not current cpu mode is in secure state.
    mrc     p15, 0, r0, c1, c1, 0   @ Read Secure Configuration Register
    teq     r0, #0x1
    beq     __error_ns

    @ Check the virtualization Extensions(VE).
    mrc     p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst     r0, #0x00001000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq     __error_ve              @ If ID_PFR[12] is equal to 0, stop here.

    @ Check the Generic Timer Extenion.
    @ TODO(wonseok): We will move the generic timer configuration into C code.
    mrc     p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst     r0, #0x00010000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq     __error_ge

    @ Configure the Generic Timer Frequency
    ldr        r0, = CFG_CNTFRQ
    mcr        p15, 0, r0, c14, c0, 0

#ifdef __CONFIG_SMP__
    @ is here on primary CPU?
    mrc	    p15, 0, r5, c0, c0, 5		@ MPIDR (ARMv7 only)
    ands    r5, r5, #0xFF
    cmp     r5, #0x0
    bne     2f
#endif

	@ Initialize BSS Section.
    ldr     r2, =__begin_bss
    ldr     r3, =__end_bss

    mov     r0, #0
1:  str     r0, [r2], #4
    cmp     r2, r3
    blo     1b

    /*
     * We assume that the CPU mode is SVC in secure world at this point.
     * Initialze procedure as below execute only CPU ID == 0.
     */
    @ Set all interrupts to be non-secure: Set GICD_IGROUPRn to 0xffffffff
    @ Get the number of GICD_IGROUPRn == GICD_TYPER.ITLinesNumber + 1.
    ldr	    r0, = CFG_GICD_BASE_PA	@ GICD base
    ldr	    r1, [r0, #0x04]			@ Type Register
    and	    r1, r1, #0x1f
    add	    r2, r0, #0x080			@ Security Register 0
    mvn	    r3, #0                  @ set r3 == 0xffffffff
1:
    str	    r3, [r2]
    sub	    r1, r1, #1
    add	    r2, r2, #4			    @ Next security register
    cmp	    r1, #-1
    bne	    1b

    @ Set GIC priority for sharing IRQ between NS and Secure worlds
    @ Before the Normal world can access the register, the Secure
    @ world MUST write a value greater than 0x80 to it.
    ldr	    r0, = CFG_GICC_BASE_PA	@ GICC base
    mov	    r1, #0x80
    str	    r1, [r0, #0x4]			@ GIC ICCPMR

    @ Give the access permission for some coprocessor to non-secure(NSACR).
2:  mrc	    p15, 0, r0, c1, c1, 2
    ldr	    r1, =0x43fff
    orr	    r0, r0, r1
    mcr	    p15, 0, r0, c1, c1, 2

    /* Initialize vector for monitor mode to enter NSHyp mode */
    ldr     r1, = __monitor_vector
    mcr     p15, 0, r1, c12, c0, 1      @ Set the vector for monitor mode

    smc     #0  @ -> trap_smc
    hvc     #0  @ -> trap_hyp_entry

    /* We are going to initialize for each core, here */
    @ DCIMVAC: invalidate data cache.
    mov     r0, #0
    mcr     p15, 0, r0, c7, c6, 1

    @ TLBIALLH: flust instruction and data TLBs. r0 will be ignored.
    mcr     p15, 4, r0, c8, c7, 0

    /* Replace the NSHyp vector table for khypervisor */
    ldr     r0, = __hvc_vector
    mcr     p15, 4, r0, c12, c0, 0

    ldr     sp, = __end_stack
    mrc     p15, 0, r5, c0, c0, 5		@ MPIDR (ARMv7 only)
    ands    r5, r5, #0xFF
    mov     r6, #HYP_STACK_SIZE
    mul     r6, r6, r5
    sub     sp, sp, r6

#ifdef __CONFIG_SMP__
    mrc	    p15, 0, r5, c0, c0, 5		@ MPIDR (ARMv7 only)
    ands    r5, r5, #0xFF
    cmp     r5, #0x0
    beq     1f
    b       enter_smp_pen
1:
#endif
    bl     init_system

	@ Never reached, here.
    b       __error

#ifdef __CONFIG_SMP__
    .globl enter_smp_pen
enter_smp_pen:
    ldr     r0, =secondary_smp_pen
    ldr     r1, [r0]
    cmp     r1, #0  @ pending
    beq     enter_smp_pen

    mcr     p15, 0, r2, c7, c11, 1 /* Clean the data cache by MVA*/
    mov     r10, #0
    mcr     p15, 0, r10, c7, c5, 0 /* Invalidate the I-cache */
    isb     /* Make sure the invalidate ops are complete */
    dsb

    bl      init_system
#endif

__error_ns:
__error_ve:
__error_ge:
__error:
    b __error
END_PROC(__start)
