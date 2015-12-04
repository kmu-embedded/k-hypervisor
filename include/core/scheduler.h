#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stdio.h>
#include "armv7_p15.h"
#include "timer.h"
#include <hvmm_types.h>
#include <vgic.h>
#include <guest_regs.h>
#include <guest.h>

extern struct guest_module _guest_module;
extern struct guest_struct *_current_guest[NUM_CPUS];
//extern struct guest_struct guests[NUM_GUESTS_STATIC];
/**
 * sched_policy_determ_next() should be used to determine next virtual
 * machin. Currently, K-Hypervisor scheduler is a round robin, so
 * it has been implemented very simply by increasing the vmid number.
 */
vmid_t sched_policy_determ_next(void);

/**
 * guest_perform_switch() perform the exchange of register from old virtual
 * to new virtual machine. Mainly, this function is called by trap and
 * the timer interrupt.
 */
hvmm_status_t guest_perform_switch(struct arch_regs *regs);

void guest_sched_start(void);
vmid_t guest_first_vmid(void);
vmid_t guest_last_vmid(void);
vmid_t guest_next_vmid(vmid_t ofvmid);
vmid_t guest_current_vmid(void);
vmid_t guest_waiting_vmid(void);
hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked);
//extern void __mon_switch_to_guest_context(struct arch_regs *regs);

//struct guest_struct get_guest(uint32_t guest_num);

void set_manually_select_vmid(vmid_t vmid);
void clean_manually_select_vmid(void);

void guest_schedule(void *pdata);
#endif /* __SCHEDULER_H__ */
