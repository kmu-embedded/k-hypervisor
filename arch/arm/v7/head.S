#include <asm/asm.h>
#include <board/rtsm-config.h>
    .syntax	unified
    .arch_extension sec
    .arch_extension virt
    .text

BEGIN_PROC(__start)
    @ Disable interrupts
    cpsid aif

    @ Check whether or not current cpu mode is in secure state.
    mrc p15, 0, r0, c1, c1, 0   @ Read Secure Configuration Register
    teq r0, #0x1
    beq __error_ns

    @ Check the virtualization Extensions(VE).
    mrc p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst r0, #0x00001000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq __error_ve              @ If ID_PFR[12] is equal to 0, stop here.

    @ Check the Generic Timer Extenion.
    @ TODO(wonseok): We will move the generic timer configuration into C code.
    mrc p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst r0, #0x00010000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq __error_ge

    @ DCIMVAC: Invalidate data cache.
    mov r0, #0
    mcr p15, 0, r0, c7, c6, 1

    @ TLBIALLH: flust instruction and data TLBs. r0 will be ignored.
    mcr p15, 4, r0, c8, c7, 0

    @ TODO(wonseok): Initialize BSS Section.

    @ Initialize CPU
    @ mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
    @ and	r4, r4, #15			@ CPU number

    /*
     * We assume that the CPU mode is SVC in secure world at this point.
     */
    @ TODO(wonseok): Replace named constrant e.g. CFG_GIC_BASE_PA + xxxx.

    @ Set all interrupts to be non-secure: Set GICD_IGROUPRn to 0xffffffff
    @ Get the number of GICD_IGROUPRn == GICD_TYPER.ITLinesNumber + 1.
    ldr	r0, = CFG_GIC_BASE_PA+0x1000	@ GICD base
    ldr	r1, [r0, #0x04]			@ Type Register
    cmp	r4, #0
    andeq	r1, r1, #0x1f
    movne	r1, #0
    add	r2, r0, #0x080			@ Security Register 0
    mvn	r3, #0                  @ set r3 == 0xffffffff
2:
    str	r3, [r2]
    sub	r1, r1, #1
    add	r2, r2, #4			@ Next security register
    cmp	r1, #-1
    bne	2b

    @ Set GIC priority for sharing IRQ between NS and Secure worlds
    @ Before the Normal world can access the register, the Secure
    @ world MUST write a value greater than 0x80 to it.
    ldr	r0, = CFG_GIC_BASE_PA+0x2000	@ GICC base
    mov	r1, #0x80
    str	r1, [r0, #0x4]			@ GIC ICCPMR

    @ Give the access permission for some coprocessor to non-secure(NSACR).
    mrc	p15, 0, r0, c1, c1, 2
    ldr	r1, =0x43fff
    orr	r0, r0, r1
    mcr	p15, 0, r0, c1, c1, 2

    @ Check current CPU ID.
    mrc p15, 0, r0, c0, c0, 5       @ MPIDR (ARMv7 only)
    @ CPU number, taking multicluster into account
    bfc r0, #24, #8
    cmp r0, #0              @ primary CPU?
    beq 2f
    b __error

2:
    // TODO(elsdy): We will be added Secondary CPU initialization, here. Because the above procedure
    // do not need to execute for every core.

    /* Initialize vector for monitor mode to enter NSHyp mode */
    ldr r1, = __monitor_vector
    mcr p15, 0, r1, c12, c0, 1      @ Set the vector for monitor mode

    smc #0  @ -> trap_smc
    hvc #0  @ -> trap_hyp_entry

    /* Replace the NSHyp vector table for khypervisor */
    @ldr r0, = __hvc_vector
    @mcr p15, 4, r0, c12, c0, 0

    bl main

	@ Never reached
    b __error

__error_ns:
__error_ve:
__error_ge:
__error:
    b __error
END_PROC(__start)
