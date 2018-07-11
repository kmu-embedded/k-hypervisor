#include <arch/armv7.h>
#include <arch/optee.h>
#include <arch/smccc.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>

#include "paging.h"

struct optee_thread threads[CONFIG_NR_OPTEE_THREAD];

void handle_optee_get_shm_config(struct core_regs *regs);
int handle_optee_return_from_rpc(struct core_regs *regs);
void handle_optee_rpc(struct core_regs *regs);

int handle_optee_smc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch (function_id) {
    case OPTEE_SMC_GET_SHM_CONFIG:
        handle_optee_get_shm_config(regs);
        return 0;
    case OPTEE_SMC_RETURN_FROM_RPC:
        if (handle_optee_return_from_rpc(regs))
            return 0;
    }

    arm_smccc_smc(regs->gpr[0], regs->gpr[1], regs->gpr[2], regs->gpr[3],
                  regs->gpr[4], regs->gpr[5], regs->gpr[6], regs->gpr[7],
                  (struct arm_smccc_res *) regs->gpr);

    if (OPTEE_SMC_RET_IS_RPC(regs->gpr[0])) {
        handle_optee_rpc(regs);
    }

    return 0;
}

void handle_optee_get_shm_config(struct core_regs *regs)
{
    static struct optee_shm shm = { -1, -1 };
    struct vmcb *vm = get_current_vm();

    if (shm.base == (uint32_t) -1) {
        arm_smccc_smc(regs->gpr[0], 0, 0, 0, 0, 0, 0, 0,
                (struct arm_smccc_res *) regs->gpr);

        shm.base = regs->gpr[1];
        shm.size = regs->gpr[2];
        shm.cached = regs->gpr[3];

        paging_add_mapping(shm.base, shm.base, MT_WRITEBACK_RW_ALLOC, shm.size);
    }

    // TODO: need alignment if CONFIG_NR_VMS is not a multiple of 2
    regs->gpr[3] = shm.cached;
    regs->gpr[2] = shm.size / CONFIG_NR_VMS;
    regs->gpr[1] = shm.base + (regs->gpr[2] * vm->vmid);
    regs->gpr[0] = OPTEE_SMC_RET_OK;

    paging_add_ipa_mapping(vm->vmem.base, shm.base, shm.base,
            MEMATTR_NORMAL_WT_CACHEABLE, 1, shm.size);
}

int handle_optee_return_from_rpc(struct core_regs *regs)
{
    uint32_t thread_id = regs->gpr[3];
    struct optee_thread *thread = &threads[thread_id];

    switch (thread->rpc) {
    case OPTEE_RPC_FUNC_ALLOC:
        thread->msg = (struct optee_msg_arg *) regs->gpr[2];
        break;
    case OPTEE_RPC_FUNC_FREE:
        thread->msg = NULL;
        break;
    case OPTEE_RPC_FUNC_DUMMY:
        if (thread->is_sleep) { // make normal world's thread to do busy waiting
            regs->gpr[0] = OPTEE_RPC_FUNC_DUMMY;
            return 1;
        } else {
            thread->msg->ret = 0;
        }
    }

    thread->rpc = -1;
    return 0;
}

void handle_optee_rpc(struct core_regs *regs)
{
    uint32_t thread_id = regs->gpr[3];
    struct optee_thread *thread = &threads[thread_id];

    thread->rpc = regs->gpr[0];

    if (thread->rpc != OPTEE_RPC_FUNC_CMD) return;

    struct optee_msg_arg *msg = thread->msg;

    switch (msg->cmd) {
    case OPTEE_RPC_CMD_WAIT_QUEUE:
        if (msg->params[0].u.val.a) { // OPTEE_RPC_CMD_WQ_WAKE
            thread = &threads[msg->params[0].u.val.b];
            thread->is_sleep = false;
        } else { // OPTEE_RPC_CMD_WQ_SLEEP
            thread->rpc = regs->gpr[0] = OPTEE_RPC_FUNC_DUMMY;
            thread->is_sleep = true;
        }
    }
}
