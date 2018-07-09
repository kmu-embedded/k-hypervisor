#ifndef __OPTEE_H__
#define __OPTEE_H__

#include <core/vm/vm.h>
#include <arch/smccc.h>

#define OPTEE_SMC_SERVICE_TRUSTED_OS 50
#define OPTEE_SMC_SERVICE_TRUSTED_OS_API 63

// OPTEE_API_FAST_CALL (prefix: 0xbf00)

#define OPTEE_SMC_CALLS_COUNT       0xbf00ff00
#define OPTEE_SMC_CALLS_UID         0xbf00ff01
#define OPTEE_SMC_CALLS_REVISION    0xbf00ff03

// OPTEE_SMC_FAST_CALL (prefix: 0xb200)

#define OPTEE_SMC_GET_OS_UUID       0xb2000000
#define OPTEE_SMC_GET_OS_REVISION   0xb2000001
#define OPTEE_SMC_GET_SHM_CONFIG    0xb2000007
#define OPTEE_SMC_L2CC_MUTEX        0xb2000008
#define OPTEE_SMC_EXCHANGE_CAP      0xb2000009
#define OPTEE_SMC_DISABLE_SHM_CACHE 0xb200000a
#define OPTEE_SMC_ENABLE_SHM_CACHE  0xb200000b
#define OPTEE_SMC_BOOT_SECONDARY    0xb200000c
#define OPTEE_SMC_VM_CREATED        0xb200000d
#define OPTEE_SMC_VM_DESTROYED      0xb200000e

// OPTEE_SMC_STD_CALL (prefix: 0x3200)

#define OPTEE_SMC_RETURN_FROM_RPC   0x32000003
#define OPTEE_SMC_CALL_WITH_ARG     0x32000004

enum optee_msg_cmd {
    OPTEE_MSG_CMD_OPEN_SESSION = 0,
    OPTEE_MSG_CMD_INVOKE_COMMAND,
    OPTEE_MSG_CMD_CLOSE_SESSION,
    OPTEE_MSG_CMD_CANCEL,
    OPTEE_MSG_CMD_REGISTER_SHM,
    OPTEE_MSG_CMD_UNREGISTER_SHM
};

// OPTEE_API_64_FAST_CALL (prefix: 0xffff)

#define OPTEE_RPC_FUNC_ALLOC     0Xffff0000
#define OPTEE_RPC_FUNC_FREE      0Xffff0002
#define OPTEE_RPC_FUNC_FOREIGN   0Xffff0004
#define OPTEE_RPC_FUNC_CMD       0Xffff0005
#define OPTEE_RPC_FUNC_DUMMY     0Xfffffff0

enum optee_rpc_cmd {
    OPTEE_RPC_CMD_LOAD_TA = 0,
    OPTEE_RPC_CMD_RPMB,
    OPTEE_RPC_CMD_FS,
    OPTEE_RPC_CMD_GET_TIME,
    OPTEE_RPC_CMD_WAIT_QUEUE,
    OPTEE_RPC_CMD_SUSPEND,
    OPTEE_RPC_CMD_SHM_ALLOC,
    OPTEE_RPC_CMD_SHM_FREE,
    OPTEE_RPC_CMD_BENCH_REG = 20
};

enum optee_rpc_cmd_wq {
    OPTEE_RPC_CMD_WQ_SLEEP = 0,
    OPTEE_RPC_CMD_WQ_WAKE
};

struct optee_msg_param_tmem {
    uint64_t buf_ptr;
    uint64_t size;
    uint64_t shm_ref;
};

struct optee_msg_param_rmem {
    uint64_t offs;
    uint64_t size;
    uint64_t shm_ref;
};

struct optee_msg_param_value {
    uint64_t a;
    uint64_t b;
    uint64_t c;
};

struct optee_msg_param {
    uint64_t attr;
    union {
        struct optee_msg_param_tmem tmem;
        struct optee_msg_param_rmem rmem;
        struct optee_msg_param_value val;
    } u;
};

struct optee_msg_arg {
    uint32_t cmd;
    uint32_t func;
    uint32_t session;
    uint32_t cancel_id;
    uint32_t pad;
    uint32_t ret;
    uint32_t ret_origin;
    uint32_t num_params;

    struct optee_msg_param params[];
};

struct optee_thread {
    vmid_t vmid; // not used yet.
    int rpc;
    bool is_sleep;

    struct optee_msg_arg *msg;
};

int handle_optee_smc(struct core_regs *regs);

#endif
