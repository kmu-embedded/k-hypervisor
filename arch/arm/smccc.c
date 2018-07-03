#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/smccc.h>
#include <core/scheduler.h>

void handle_optee_smc(uint32_t *a);

int handle_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch(function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    //TODO: Add trusted OS related case, here.
    default:
        handle_optee_smc(regs->gpr);
    }

    return 0;
}

//TODO: Move below to separated c file.

#define NR_OPTEE_THREAD 8

#define FN_GET_SHM_CONFIG   0xb2000007
#define FN_RETURN_FROM_RPC  0x32000003

#define RPC_FUNC_ALLOC      0xffff0000
#define RPC_FUNC_CMD        0xffff0005
#define RPC_FUNC_WQ_DUMMY   0xfffffff0

#define RPC_CMD_WAIT_QUEUE  4

struct optee_msg_arg {
    uint32_t cmd;
    uint32_t func;
    uint32_t session;
    uint32_t cancel_id;
    uint32_t pad;
    uint32_t ret;
    uint32_t ret_origin;
    uint32_t num_params;

//    struct optee_msg_param params[];
    uint64_t attr;
    uint64_t a;
    uint64_t b;
    uint64_t c;
};

struct optee_thread {
    vmid_t vmid; // not used yet.
    int rpc;
    bool is_sleep;

    struct optee_msg_arg *msg;
} threads[NR_OPTEE_THREAD];

void handle_optee_smc(uint32_t *a)
{
    uint32_t function_id = a[0];

    // TODO(jigi.kim): add case for open session (for thread-vm mapping)
    // TODO(jigi.kim): make it with switch statement.
    if (function_id == FN_RETURN_FROM_RPC) {
        uint32_t shm_pa = a[2];
        uint32_t thread_id = a[3];

        struct optee_thread *thread = &threads[thread_id];

        if (thread->rpc == RPC_FUNC_ALLOC) {
            thread->msg = (struct optee_msg_arg *) shm_pa;
            thread->rpc = -1;
        } else if (thread->rpc == RPC_FUNC_WQ_DUMMY) {
            if (thread->is_sleep) { // make thread to do busy waiting
                a[0] = a[1] = RPC_FUNC_WQ_DUMMY;
                return;
            }

            thread->msg->ret = 0;
            thread->rpc = -1;
        }
    }

    arm_smccc_smc(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7],
            (struct arm_smccc_res *) a);

    // TODO(jigi.kim): make it with switch statement.
    if (function_id == FN_GET_SHM_CONFIG) {
        vcpuid_t vcpuid = get_current_vcpuid();

        uint32_t base_addr = a[1];
        uint32_t offset = a[2];

        a[2] = offset/CONFIG_NR_VMS;
        a[1] = base_addr + (a[2] * vcpuid);
    }

    // TODO(jigi.kim): make it with switch statement.
    if (a[0] == RPC_FUNC_ALLOC) {
        struct optee_thread *thread = &threads[a[3]];

        thread->rpc = RPC_FUNC_ALLOC;
    } else if (a[0] == RPC_FUNC_CMD) {
        struct optee_thread *thread = &threads[a[3]];
        struct optee_msg_arg *msg = thread->msg;

        if (msg->cmd == RPC_CMD_WAIT_QUEUE) { 
            if (msg->a) { // wake
                thread->rpc = RPC_CMD_WAIT_QUEUE;
                thread = &threads[msg->b];
                thread->is_sleep = false;
            } else { // sleep
                thread->is_sleep = true;
                thread->rpc = RPC_FUNC_WQ_DUMMY;
                a[0] = RPC_FUNC_WQ_DUMMY;
            }
        }
    }
}
