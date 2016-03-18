#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <core/vm/vcpu.h>
#include "../types.h"
#include <arch_regs.h>

#define GUEST_VERBOSE_ALL       0xFF
#define GUEST_VERBOSE_LEVEL_0   0x01
#define GUEST_VERBOSE_LEVEL_1   0x02
#define GUEST_VERBOSE_LEVEL_2   0x04
#define GUEST_VERBOSE_LEVEL_3   0x08
#define GUEST_VERBOSE_LEVEL_4   0x10
#define GUEST_VERBOSE_LEVEL_5   0x20
#define GUEST_VERBOSE_LEVEL_6   0x40
#define GUEST_VERBOSE_LEVEL_7   0x80

void sched_init();
//sched_register_vcpu?
int sched_vcpu_register(vcpuid_t vcpuid);
//sched_unregister_vcpu?
int sched_vcpu_unregister();
int sched_vcpu_attach(vcpuid_t vcpuid);
int sched_vcpu_detach();
void do_schedule(void *pdata, uint32_t *delay_tick);

void sched_start(void);
vmid_t get_current_vcpuid(void);

hvmm_status_t sched_perform_switch(struct core_regs *regs);
hvmm_status_t guest_switchto(vmid_t vmid);

struct running_vcpus_entry_t {
    struct list_head head;
    vcpuid_t vcpuid;
};

extern struct list_head __running_vcpus[];

#endif /* __SCHEDULER_H__ */
