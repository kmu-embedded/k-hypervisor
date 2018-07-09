#include <arch/armv7.h>
#include <arch/optee.h>
#include <arch/smccc.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>

struct optee_thread threads[NR_OPTEE_THREAD];

int handle_optee_smc(struct core_regs *regs)
{
    uint32_t *a = regs->gpr;
    uint32_t function_id = a[0];

    // TODO(jigi.kim): add case for open session (for thread-vm mapping)
    // TODO(jigi.kim): make it with switch statement.
    if (function_id == OPTEE_SMC_RETURN_FROM_RPC) {
        uint32_t shm_pa = a[2];
        uint32_t thread_id = a[3];

        struct optee_thread *thread = &threads[thread_id];

        if (thread->rpc == OPTEE_RPC_FUNC_ALLOC) {
            thread->msg = (struct optee_msg_arg *) shm_pa;
            thread->rpc = -1;
        } else if (thread->rpc == OPTEE_RPC_FUNC_DUMMY) {
            if (thread->is_sleep) { // make thread to do busy waiting
                a[0] = a[1] = OPTEE_RPC_FUNC_DUMMY;
                return 0;
            }

            thread->msg->ret = 0;
            thread->rpc = -1;
        }
    }

    arm_smccc_smc(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7],
            (struct arm_smccc_res *) a);

    // TODO(jigi.kim): make it with switch statement.
    if (function_id == OPTEE_SMC_GET_SMC_CONFIG) {
        vcpuid_t vcpuid = get_current_vcpuid();

        uint32_t base_addr = a[1];
        uint32_t offset = a[2];

        a[2] = offset/CONFIG_NR_VMS;
        a[1] = base_addr + (a[2] * vcpuid);
    }

    // TODO(jigi.kim): make it with switch statement.
    if (a[0] == OPTEE_RPC_FUNC_ALLOC) {
        struct optee_thread *thread = &threads[a[3]];

        thread->rpc = OPTEE_RPC_FUNC_ALLOC;
    } else if (a[0] == OPTEE_RPC_FUNC_CMD) {
        struct optee_thread *thread = &threads[a[3]];
        struct optee_msg_arg *msg = thread->msg;

        if (msg->cmd == OPTEE_RPC_CMD_WAIT_QUEUE) { 
            if (msg->params[0].u.val.a) { // wake
                thread->rpc = OPTEE_RPC_CMD_WAIT_QUEUE;
                thread = &threads[msg->params[0].u.val.b];
                thread->is_sleep = false;
            } else { // sleep
                thread->is_sleep = true;
                thread->rpc = a[0] = OPTEE_RPC_FUNC_DUMMY;
            }
        }
    }

    return 0;
}
