/* Refer to ARM Synchronization Primitives(DHT0008A) */

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
