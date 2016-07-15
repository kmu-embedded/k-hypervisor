/* Refer to ARM Synchronization Primitives(DHT0008A) */

#ifdef CONFIG_SMP
#define UNLOCKED  0xFFFFFFFF

    .global init_mutex
init_mutex:
    mov     r1, #UNLOCKED
    str     r1, [r0]

    bx      lr

    .global lock_mutex
lock_mutex:
    @ is mutex locked?
    ldrex   r1, [r0]
    cmp     r1, #UNLOCKED

    wfene
    bne     lock_mutex

    @ attempt to lock mutex
    mrc	    p15, 0, r1, c0, c0, 5		@ MPIDR (ARMv7 only)
    mov     r2, #0x03
    orr     r2, r2, #0x100              @ For cluster0/1
    and     r1, r1, r2
    strex   r2, r1, [r0]
    cmp     r2, #0x0
    bne     lock_mutex

    dmb

    bx      lr

    .global unlock_mutex
unlock_mutex:
    @ is this CPU owned the mutex?
    mrc	    p15, 0, r1, c0, c0, 5		@ MPIDR (ARMv7 only)
    mov     r2, #0x03
    orr     r2, r2, #0x100              @ For cluster0/1
    and     r1, r1, r2
    ldr     r2, [r0]
    cmp     r1, r2
    movne   r0, #0x1
    bxne    lr

    @ unlock mutex
    dmb

    mov     r1, #UNLOCKED
    str     r1, [r0]

    dsb

    sev

    mov     r0, #0x0
    bx      lr

    .global is_mutex_locked
is_mutex_locked:
    ldr     r0, [r0]
    cmp     r0, #UNLOCKED
    moveq   r0, #0x0
    movne   r0, #0x0
    bx      lr

#else // ifdef CONFIG_UP

#define UNLOCKED  0
#define LOCKED    1
    .global lock_mutex
lock_mutex:
    ldr     r1, =LOCKED
1:  ldrex   r2, [r0]
    cmp     r2, r1
    beq     2f
    strexne r2, r1, [r0]
    cmpne   r2, #1
    beq     1b
    # Lock acquired
    dmb
    bx      lr

2:  b       1b

    .global unlock_mutex
unlock_mutex:
    ldr     r1, =UNLOCKED
    dmb
    str     r1, [r0]
    bx      lr
#endif
