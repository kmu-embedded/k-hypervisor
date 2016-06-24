#include <asm/macro.h>
#include <config.h>
#include <arch/armv7/cp15.h>
#include <generated/autoconf.h>

#define PRINT(_s)       \
    adr   r0, 98f ; \
    bl    puts    ; \
    b     99f     ; \
98: .asciz _s     ; \
    .align 2      ; \
99:

    .syntax	unified
    .arch_extension sec
    .arch_extension virt

BEGIN_PROC(__start)
    @ Disable interrupts
	cpsid   aif

    @read_cp32(r5, MPIDR)
	@mov		r4, #0x01
@	orr		r4, r4, #0x100
@	orr		r4, r4, #0x80000000
@	cmp     r5, r4
@
@	beq		1f
@
@	mov		r4, #0x100
@	orr		r4, r4, #0x80000000
@	cmp     r5, r4
@
@	beq		1f
@
@	wfe

1:	ldr   r11, =0x12c20000       /* r11 := UART base address */
    teq   r12, #0                /* Boot CPU sets up the UART too */
    bleq  init_uart
    @PRINT("- CPU ")
    @mov   r0, r7
    @bl    putn
    @PRINT(" booting -\r\n")


	@ Check HYP mode
	mrs		r0, cpsr_all
	and		r0, r0, #0x1F
	mov		r8, #0x1A
	cmp		r0, r8
	beq		3f

    @PRINT(" Check Secure-\r\n")
    @ Check whether or not current cpu mode is in secure state.
    read_cp32(r0, SCR)
    teq     r0, #0x1
    @ TODO(wonseok): if current cpu is in non-secure,
    @ we have to check current cpu mode is HYP or not.
    beq     __error_ns

    @PRINT(" Check VE -\r\n")
    @ Check the virtualization Extensions(VE).
    read_cp32(r0, ID_PFR1)
    tst     r0, #0x00001000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq     __error_ve              @ If ID_PFR[12] is equal to 0, stop here.

    @ Check the Generic Timer Extenion.
    read_cp32(r0, ID_PFR1)
    tst     r0, #0x00010000         @ Compare the value in ID_PFR[12] to 1 or 0.
    beq     __error_ge

    @ Configure the Generic Timer Frequency
    ldr        r0, = CFG_CNTFRQ
    write_cp32(r0, CNTFRQ)

    /*
     * We assume that the CPU mode is SVC in secure world at this point.
     * Initialze procedure as below execute only CPU ID == 0.
     */
    @ Set all interrupts to be non-secure: Set GICD_IGROUPRn to 0xffffffff
    @ Get the number of GICD_IGROUPRn == GICD_TYPER.ITLinesNumber + 1.
    ldr	    r0, = CFG_GICD_BASE_PA	@ GICD base
    ldr	    r1, [r0, #0x04]			@ Type Register
    and	    r1, r1, #0x1f			@ set r1 == GICD_TYPER.ITLinesNumber + 1
    add	    r2, r0, #0x080			@ Security Register 0
    mvn	    r3, #0                  @ set r3 == 0xffffffff

	@ =========== Init IGROUPR0 should be seperated to init secondary cores
1:
    str	    r3, [r2]
#ifdef CONFIG_SMP
    mrc	    p15, 0, r5, c0, c0, 5		@ MPIDR (ARMv7 only)
	mov		r4, #BOOTABLE_CPUID
	orr		r4, r4, #0x80000000
	cmp     r5, r4
    bne     2f
#endif
    sub	    r1, r1, #1
    add	    r2, r2, #4			    @ Next security register
    cmp	    r1, #-1
    bne	    1b

	@ =========== This GIC Init secondary cores also conducted

    @ Set GIC priority for sharing IRQ between NS and Secure worlds
    @ Before the Normal world can access the register, the Secure
    @ world MUST write a value greater than 0x80 to it.
2:  ldr	    r0, = CFG_GICC_BASE_PA	@ GICC base
    mov	    r1, #0x80
    str	    r1, [r0, #0x4]			@ GIC ICCPMR

    @ Give the access permission for some coprocessor to non-secure(NSACR).
    read_cp32(r0, NSACR)
    ldr	    r1, =0x43fff
    orr	    r0, r0, r1
    write_cp32(r0, NSACR)

    /* Initialize vector for monitor mode to enter NSHyp mode */
    ldr     r1, = __monitor_vector
    write_cp32(r1, MVBAR)

    smc     #0  @ -> trap_smc
    hvc     #0  @ -> trap_hyp_entry


3:
    @PRINT(" IN NORMAL WORLD-\r\n")
    /* We are going to initialize for each core, here */
    @ DCIMVAC: invalidate data cache.
    mov     r0, #0
    write_cp32(r0, DCIMVAC)

    @ TLBIALLH: flust instruction and data TLBs. r0 will be ignored.
    write_cp32(r0, TLBIALLH)

    ldr     sp, = __end_stack
    read_cp32(r5, MPIDR)
	and     r2, r5, #0x100      @ r2 == CLUSTER ID
	lsr     r2, r2, 8
	mov     r2, r2, lsl #2      @ cluster * 4
	and     r3, r5, #0x03       @ r3 == CPU ID
	add     r5, r2, r3          @ real cpuid
    mov     r6, #HYP_STACK_SIZE
    mul     r6, r6, r5
    sub     sp, sp, r6

#ifdef CONFIG_SMP
    read_cp32(r5, MPIDR)
	mov		r4, #BOOTABLE_CPUID
	orr		r4, r4, #0x80000000
	cmp     r5, r4
    beq     1f
    b       enter_smp_pen
1:
#endif

	@ Initialize BSS Section.
    ldr     r2, =__begin_bss
    ldr     r3, =__end_bss

    mov     r0, #0
1:  str     r0, [r2], #4
    cmp     r2, r3
    blo     1b

    bl     init_cpu

	@ Never reached, here.
    b       __error

#ifdef CONFIG_SMP
    .globl enter_smp_pen
enter_smp_pen:
    ldr     r0, =secondary_smp_pen
    ldr     r1, [r0]
    cmp     r1, #0  @ pending
    beq     enter_smp_pen

    write_cp32(r2, DCCMVAU)
    mov     r10, #0
    write_cp32(r10, ICIALLU)
    isb     /* Make sure the invalidate ops are complete */
    dsb

	@PRINT(" SECOUNDARY CPU ")
    bl      init_secondary_cpus

#endif

__error_ns:
__error_ve:
__error_ge:
__error:
    b __error

#include "exynos4210-uart.h"
.macro early_uart_ready rb rc
1:
    ldr   \rc, [\rb, #UTRSTAT]   /* <- UTRSTAT (Flag register) */
    tst   \rc, #UTRSTAT_TXFE     /* Check BUSY bit */
    beq   1b                     /* Wait for the UART to be ready */
.endm

/* Exynos 5 UART transmit character
 * rb: register which contains the UART base address
 * rt: register which contains the character to transmit */
.macro early_uart_transmit rb rt
    str   \rt, [\rb, #UTXH]      /* -> UTXH (Data Register) */
.endm

/* Print a character on the UART - this function is called by C
 * r0: character to print */
BEGIN_PROC(early_putch)
    ldr   r1, =0x12c20000  /* r1 := VA UART base address */
    early_uart_ready r1, r2
    early_uart_transmit r1, r0
    mov   pc, lr

    /* Flush the UART - this function is called by C */
BEGIN_PROC(early_flush)
    ldr   r1, =0x12c20000  /* r1 := VA UART base address */
    early_uart_ready r1, r2
    mov   pc, lr

init_uart:
    adr   r0, 1f
    b     puts                  /* Jump to puts */
    1:      .asciz "- UART enabled -\r\n"
    .align 4

    /* Print early debug messages.
     * r0: Nul-terminated string to print.
     * r11: Early UART base address
     * Clobbers r0-r1 */
puts:
    early_uart_ready r11, r1
    ldrb  r1, [r0], #1           /* Load next char */
    teq   r1, #0                 /* Exit on nul */
    moveq pc, lr
    early_uart_transmit r11, r1
    b puts

    /* Print a 32-bit number in hex.  Specific to the PL011 UART.
     * r0: Number to print.
     * r11: Early UART base address
     * Clobbers r0-r3 */
putn:
    adr   r1, hex
    mov   r3, #8
    1:
    early_uart_ready r11, r2
    and   r2, r0, #0xf0000000    /* Mask off the top nybble */
    ldrb  r2, [r1, r2, lsr #28]  /* Convert to a char */
    early_uart_transmit r11, r2
    lsl   r0, #4                 /* Roll it through one nybble at a time */
    subs  r3, r3, #1
    bne   1b
    mov   pc, lr

    hex:    .ascii "0123456789abcdef"
    .align 2

END_PROC(__start)

