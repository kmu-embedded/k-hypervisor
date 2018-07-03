#ifndef __OPTEE_H__
#define __OPTEE_H__

#include <core/vm/vm.h>

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

void handle_optee_smc(struct core_regs *regs);

#endif
