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

int sched_vcpu_register(vcpuid_t vcpuid, uint32_t pcpu);
int sched_vcpu_register_to_current_pcpu(vcpuid_t vcpuid);
int sched_vcpu_unregister(vcpuid_t vcpuid, uint32_t pcpu);

int sched_vcpu_attach(vcpuid_t vcpuid, uint32_t pcpu);
int sched_vcpu_attach_to_current_pcpu(vcpuid_t vcpuid);
int sched_vcpu_detach(vcpuid_t vcpuid, uint32_t pcpu);

void do_schedule(void *pdata, uint64_t *expiration);

void sched_start(void);
vmid_t get_current_vcpuid(void);
vcpuid_t get_current_vcpuidx(void);
struct vcpu *get_current_vcpu(void);
struct vmcb *get_current_vm(void);

hvmm_status_t guest_switchto(vmid_t vmid);
hvmm_status_t sched_perform_switch(struct core_regs *regs);

struct running_vcpus_entry_t {
    struct list_head head;
    vcpuid_t vcpuid;
};

extern struct list_head __running_vcpus[];

#endif /* __SCHEDULER_H__ */
