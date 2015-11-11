.global mon_init
	mon_init:
	push {r0 - r2, lr}
	@ Setup Exception Vector
	ldr r1, = monitor_secure_vectors
	mcr p15, 0, r1, c12, c0, 1      @ Monitor vector base address
	@ regular return
	pop {r0 - r2, pc}
.type mon_init, % function

.global mon_to_hyp
mon_to_hyp:
	smc #0  @ -> trap_smc
	hvc #0  @ -> trap_hyp_entry
	/* Place the right Hyp exception vector for the first time */
	ldr r0, = hyp_init_vectors
	mcr p15, 4, r0, c12, c0, 0

#ifdef _SMP_
	@ Check CPU nr again
	mrc p15, 0, r0, c0, c0, 5       @ MPIDR (ARMv7 only)
	@ CPU number, taking multicluster into account
	bfc r0, #24, #8
	cmp r0, #0              @ primary CPU?
	beq 2f
	@
	@ Secondary CPUs
	@
	b enter_smp_pen
2:
#endif
	bl main
	@ never come back
	1:
	b 1b
.type mon_to_hyp, % function

.global __mon_switch_to_guest_context
__mon_switch_to_guest_context:    @ struct arch_regs *r0
	ldmfd    r0!, {r1, r3, lr}
	msr     spsr_hyp, r1
	msr     elr_hyp, r3
	ldm     r0, {r0-r12}
	clrex
	eret
.type __mon_switch_to_guest_context, %function
