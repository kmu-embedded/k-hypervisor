#include <arch/arm/rtsm-config.h>

    .syntax	unified
    .arch_extension sec
    .arch_extension virt
    .text

    .align 5
    .globl	__start
__start:
    cpsid aif                   @ Disable the all interrupts

    // Check the virtualization Extensions(VE)
    mrc p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst r0, #0x00001000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq __error_ve              @ If ID_PFR[12] is equal to 0, stop here.

    // Check the Generic Timer Extenion
    // TODO(wonseok): We will move the generic timer configuration into C code.
    mrc p15, 0, r0, c0, c1, 1   @ Read a Processor Feature Register 1(ID_PFR1).
    tst r0, #0x00010000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq __error_ge

    // Configure the Generic Timer Frequency
    ldr	r0, = CFG_CNTFRQ
    mcr	p15, 0, r0, c14, c0, 0

    // Initialize CPU
    mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
    and	r4, r4, #15			@ CPU number

    /*
     * We assume that the CPU mode is SVC in secure world at this point.
     */

    // Set all interrupts to be non-secure
    ldr	r0, = CFG_GIC_BASE_PA+0x1000	@ Dist GIC base
    ldr	r1, [r0, #0x04]			@ Type Register
    cmp	r4, #0
    andeq	r1, r1, #0x1f
    movne	r1, #0
    add	r2, r0, #0x080			@ Security Register 0
    mvn	r3, #0
2:
    str	r3, [r2]
    sub	r1, r1, #1
    add	r2, r2, #4			@ Next security register
    cmp	r1, #-1
    bne	2b

    // Set GIC priority mask bit [7] = 1
    ldr	r0, = CFG_GIC_BASE_PA+0x2000	@ CPU GIC base
    mov	r1, #0x80
    str	r1, [r0, #0x4]			@ GIC ICCPMR

    // Set NSACR to allow coprocessor access from non-secure
    mrc	p15, 0, r0, c1, c1, 2
    ldr	r1, =0x43fff
    orr	r0, r0, r1
    mcr	p15, 0, r0, c1, c1, 2

    @ Check CPU nr again
    mrc p15, 0, r0, c0, c0, 5       @ MPIDR (ARMv7 only)
    @ CPU number, taking multicluster into account
    bfc r0, #24, #8
    cmp r0, #0              @ primary CPU?
    beq 2f
    b __error

2:
    // UART initialisation (38400 8N1)
    ldr	r0, =0x1c090000			@ UART base (Versatile Express)
    mov	r1, #0x10			@ ibrd
    str	r1, [r0, #0x24]
    mov	r1, #0xc300
    orr	r1, #0x0001			@ cr
    str	r1, [r0, #0x30]

    /*
     * Initialize UART1, UART2 and UART3 for access from guests, as a workaround,
     * since initialization sequence in C, in guest, is not clear a the moment
     * TODO(wonseok): The UART-related code will be moved into C
     */
    ldr	r0, =0x1c0a0000			@ UART base (Versatile Express)
    mov	r1, #0x10			@ ibrd
    str	r1, [r0, #0x24]
    mov	r1, #0xc300
    orr	r1, #0x0001			@ cr
    str	r1, [r0, #0x30]

    ldr	r0, =0x1c0b0000			@ UART base (Versatile Express)
    mov	r1, #0x10			@ ibrd
    str	r1, [r0, #0x24]
    mov	r1, #0xc300
    orr	r1, #0x0001			@ cr
    str	r1, [r0, #0x30]

    ldr	r0, =0x1c0c0000			@ UART base (Versatile Express)
    mov	r1, #0x10			@ ibrd
    str	r1, [r0, #0x24]
    mov	r1, #0xc300
    orr	r1, #0x0001			@ cr
    str	r1, [r0, #0x30]

    /* Initialize bss section */
    ldr  r2, =__begin_bss
    ldr  r3, =__end_bss

    mov  r0, #0
1:  str  r0, [r2], #4    @ clear bss
    cmp  r2, r3
    blo  1b

// TODO(elsdy): We will be added Secondary CPU initialization, here. Because the above procedure
// do not need to execute for every core.

    /* Initialize vector for monitor mode to enter NSHyp mode */
    ldr r1, = __mon_vector
    mcr p15, 0, r1, c12, c0, 1      @ Set the vector for monitor mode

    smc #0  @ -> trap_smc
    hvc #0  @ -> trap_hyp_entry

    /* Replace the NSHyp vector table for khypervisor*/
    ldr r0, = __hvc_vector
    mcr p15, 4, r0, c12, c0, 0
    bl init_khypervisor

	@ Never reached
    b __error

__error_ve:
__error_ge:
__error:
    wfe
    b __error

    /*
_begin_bss:
    .word   __begin_bss
_end_bss:
    .word   __end_bss
    */
