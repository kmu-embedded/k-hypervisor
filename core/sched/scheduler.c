#include <arch/armv7.h>
#include <config.h>
#include <core/timer.h>
#include <arch/irq.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>
#include <core/context_switch.h>
#include <core/scheduler.h>
#include <core/sched/scheduler_skeleton.h>

#include <debug.h>
#include <stdlib.h>
#include <stdio.h>
#include <lib/list.h>

struct scheduler *sched[NR_CPUS];

void sched_init() /* TODO: const struct sched_config const* sched_config)*/
{
    /* Check scheduler config */
    /* TODO:(igkang) choose policy based on config */

    /* Allocate and initialize data */
    uint32_t pcpu;
    for (pcpu = 0; pcpu < NR_CPUS; pcpu++) {
        const struct sched_policy *p = &sched_rr;

        struct scheduler *s
            = sched[pcpu]
            = (struct scheduler *) malloc(sizeof(struct scheduler));

        s->policy = p;
        s->policy->init(s);

        s->current_vcpuid = VCPUID_INVALID;
        s->next_vcpuid = VCPUID_INVALID;
        s->current_vcpu = NULL;
        s->current_vm = NULL;

        LIST_INITHEAD(&s->standby_entries);
        LIST_INITHEAD(&s->inflight_entries);
    }
}

/* TODO:(igkang) context switching related fucntions should be redesigned
 *   - perform_switch
 *   - switchto
 *   - do_context_switch
 */

hvmm_status_t sched_perform_switch(struct core_regs *regs)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t pcpu = smp_processor_id();
    struct core_regs *param_regs = regs;

    struct scheduler *const s = sched[pcpu];

    /*
     * If the scheduler is not already running, launch default
     * first guest. It occur in initial time.
     */
    if (s->current_vcpuid == VCPUID_INVALID) {
        debug_print("context: launching the first guest\n");
        param_regs = NULL;
    }

    /* Only if not from Hyp */
    if (s->next_vcpuid != VCPUID_INVALID) {
        vcpuid_t previous = VCPUID_INVALID;
        vcpuid_t next = VCPUID_INVALID;

        debug_print("[sched] curr:%x next:%x\n", s->current_vcpuid, s->next_vcpuid);

        /* We do the things in this way before do_context_switch()
         *  as we will not come back here on the first context switching */
        previous = s->current_vcpuid;
        next = s->next_vcpuid;

        s->current_vcpuid = s->next_vcpuid;
        s->next_vcpuid = VCPUID_INVALID;

        s->current_vcpu = vcpu_find(s->current_vcpuid);
        s->current_vm = vm_find(s->current_vcpu->vmid);

        do_context_switch(previous, next, param_regs);
        /* MUST NOT COME BACK HERE IF regs == NULL */

        return HVMM_STATUS_SUCCESS;
    }

    return result;
}

hvmm_status_t switch_to(vcpuid_t vcpuid)
{
    hvmm_status_t result = HVMM_STATUS_IGNORED;
    uint32_t pcpu = smp_processor_id();

    /* TODO:(igkang) check about below comment */
    /* valid and not current vcpuid, switch */
    sched[pcpu]->next_vcpuid = vcpuid;
    result = HVMM_STATUS_SUCCESS;

    return result;
}

/* Switch to the first guest */
void sched_start(void)
{
    uint32_t pcpu = smp_processor_id();
    struct scheduler *const s = sched[pcpu];
    struct vcpu *vcpu = NULL;

    debug_print("[hyp] switch_to_initial_guest:\n");

    /* Select the first guest context to switch to. */
    uint64_t expiration = 0;
    uint32_t vcpuid = VCPUID_INVALID;

    /* NOTE : Hanging on while if there's no vcpu to schedule on scheduler */
    while (vcpuid == VCPUID_INVALID) {
        vcpuid = s->policy->do_schedule(s, &expiration);
    }
    vcpu = vcpu_find(vcpuid);

    tm_register_timer(&s->timer, do_schedule);
    tm_set_timer(&s->timer, expiration, true);
    /* timer just started */

    switch_to(vcpu->vcpuid);
    sched_perform_switch(NULL);
}

/* Current vCPU's ID - in perspective of Hypervisor */
vcpuid_t get_current_vcpuid(void)
{
    uint32_t pcpu = smp_processor_id();

    return sched[pcpu]->current_vcpuid;
}

/* Current vCPU's ID - in perspective of VM (SMP) */
vcpuid_t get_current_vcpuidx(void)
{
    uint32_t pcpu = smp_processor_id();
    return sched[pcpu]->current_vcpu->id;
}

struct vcpu *get_current_vcpu(void)
{
    uint32_t pcpu = smp_processor_id();
    return sched[pcpu]->current_vcpu;
}

struct vmcb *get_current_vm(void)
{
    uint32_t pcpu = smp_processor_id();
    return sched[pcpu]->current_vm;
}

/* Find a entry of given vcpuid in given scheduler instance */
static inline struct sched_entry *find_entry(struct scheduler *s, vcpuid_t vcpuid)
{
    struct sched_entry *entry = NULL;

    struct sched_entry *e = NULL;
    list_for_each_entry(struct sched_entry, e,
                        &s->standby_entries, head_standby) {
        if (e->vcpuid == vcpuid) {
            entry = e;
            break;
        }
    }

    return entry;
}

/* TODO:(igkang) Rename fucntions (vcpu_VERB to VERB_vcpu) */
/**
 * Register a vCPU to a scheduler
 *
 * You have to call sched_vcpu_attach() to \
 * run a vcpu by adding it to runqueue, additionally.
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @param pcpuid ID of physical CPU
 * @return
 */
int sched_vcpu_register(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct scheduler *const s = sched[pcpu];
    struct sched_entry *new;

    new = (struct sched_entry *) malloc(
              sizeof(struct sched_entry) + s->policy->size_entry_extra);

    LIST_INITHEAD(&new->head_standby);
    LIST_INITHEAD(&new->head_inflight);
    new->state = SCHED_DETACHED;
    new->vcpuid = vcpuid;

    LIST_ADDTAIL(&new->head_standby, &s->standby_entries);
    s->policy->register_vcpu(s, new);

    /* NOTE(casionwoo) : Return the ID of physical CPU that vCPU is assigned */
    return pcpu;
}


int sched_vcpu_register_to_current_pcpu(vcpuid_t vcpuid)
{
    int pcpu = sched_vcpu_register(vcpuid, smp_processor_id());
    /* NOTE(casionwoo) : Return the ID of physical CPU that vCPU is assigned */
    return pcpu;
}

/**
 * Unregister a vCPU from a scheduler
 *
 * You have to detach vcpu first.
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_unregister(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct scheduler *const s = sched[pcpu];
    struct sched_entry *target = NULL;
    target = find_entry(s, vcpuid);

    /* TODO:(igkang) need to check before action */

    s->policy->unregister_vcpu(s, target);
    LIST_DELINIT(&target->head_standby);

    return 0;
}

/**
 * Attach a vcpu to scheduler runqueue
 *
 * You have to register the vcpu first
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
// TODO(igkang): add return type to vcpu's state
int sched_vcpu_attach(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct scheduler *const s = sched[pcpu];
    struct sched_entry *target = NULL;

    target = find_entry(s, vcpuid);

    /* TODO:(igkang) Name the return value constants. */
    if (target == NULL) {
        return 255;    /* error: not standby */
    }

    if (target->state != SCHED_DETACHED) {
        return 254;    /* error: already inflight */
    }

    target->state = SCHED_WAITING;

    s->policy->attach_vcpu(s, target);
    LIST_ADDTAIL(&target->head_inflight, &s->inflight_entries);

    /* NOTE(casionwoo) : Return the ID of physical CPU that vCPU is assigned */
    return pcpu;
}

int sched_vcpu_attach_to_current_pcpu(vcpuid_t vcpuid)
{
    int pcpu = sched_vcpu_attach(vcpuid, smp_processor_id());
    /* NOTE(casionwoo) : Return the ID of physical CPU that vCPU is assigned */
    return pcpu;
}

/**
 * Detach a vcpu from scheduler runqueue
 *
 * @param shed A scheduler definition
 * @param vcpuid ID of vCPU
 * @return
 */
int sched_vcpu_detach(vcpuid_t vcpuid, uint32_t pcpu)
{
    struct scheduler *const s = sched[pcpu];
    struct sched_entry *target = NULL;
    target = find_entry(s, vcpuid);

    /* TODO:(igkang) need to check before action */

    s->policy->detach_vcpu(s, target); /* !@#$ */
    LIST_DELINIT(&target->head_inflight);

    return 0;
}

/**
 * Main scheduler routine
 *
 * @param
 * @return
 */
void do_schedule(void *pdata, uint64_t *expiration)
{
    /* TODO:(igkang) function type(return/param) should be renewed */
    uint32_t pcpu = smp_processor_id();
    struct scheduler *const s = sched[pcpu];
    int next_vcpuid;

    /* determine next vcpu to be run by calling scheduler.do_schedule() */
    /* Also sets timer expiration for next scheduler work */
    next_vcpuid = s->policy->do_schedule(s, expiration);

    /* update vCPU's running time */

    /* manipulate variables to cause context switch */
    switch_to(next_vcpuid);
    sched_perform_switch(pdata);
}
