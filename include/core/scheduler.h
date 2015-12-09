#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <hvmm_types.h>
#include <vcpu_regs.h>
#include <vcpu.h>

#define GUEST_VERBOSE_ALL       0xFF
#define GUEST_VERBOSE_LEVEL_0   0x01
#define GUEST_VERBOSE_LEVEL_1   0x02
#define GUEST_VERBOSE_LEVEL_2   0x04
#define GUEST_VERBOSE_LEVEL_3   0x08
#define GUEST_VERBOSE_LEVEL_4   0x10
#define GUEST_VERBOSE_LEVEL_5   0x20
#define GUEST_VERBOSE_LEVEL_6   0x40
#define GUEST_VERBOSE_LEVEL_7   0x80

struct vcpu *_current_guest[NUM_CPUS];

void sched_init();
vmid_t sched_policy_determ_next(void);
hvmm_status_t guest_perform_switch(struct core_regs *regs);

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
