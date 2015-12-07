#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <hvmm_types.h>
#include <guest_hw.h>
#include <vcpu.h>

struct vcpu *_current_guest[NUM_CPUS];

vmid_t sched_policy_determ_next(void);
hvmm_status_t guest_perform_switch(struct arch_regs *regs);

void guest_sched_start(void);
vmid_t guest_first_vmid(void);
vmid_t guest_last_vmid(void);
vmid_t guest_next_vmid(vmid_t ofvmid);
vmid_t guest_current_vmid(void);
vmid_t guest_waiting_vmid(void);
hvmm_status_t guest_switchto(vmid_t vmid, uint8_t locked);

void set_manually_select_vmid(vmid_t vmid);
void clean_manually_select_vmid(void);

void guest_schedule(void *pdata);
#endif /* __SCHEDULER_H__ */
