#ifndef __SMCCC_H__
#define __SMCCC_H__

#include <arch/armv7.h>

#define SMCCC_FN_CALL_TYPE(fn)  (((fn) & 0x80000000) >> 31)
#define SMCCC_FN_CONV(fn)       (((fn) & 0x40000000) >> 30)
#define SMCCC_FN_SERVICE(fn)    (((fn) & 0x3f000000) >> 24)
#define SMCCC_FN_NUM(fn)        ((fn) & 0x0000FFFF)

enum smccc_fn_service_range {
    SMCCC_SERVICE_ARCH = 0,
    SMCCC_SERVICE_CPU,
    SMCCC_SERVICE_SIP,
    SMCCC_SERVICE_OEM,
    SMCCC_SERVICE_STANDARD,
    SMCCC_SERVICE_HYPERVISOR,

    // SMCCC_SERVICE_RESERVED = 7,
    // SMCCC_SERVICE_RESERVED_END = 47,

    SMCCC_SERVICE_TRUSTED_APP = 48,
    SMCCC_SERVICE_TRUSTED_APP_END = 49,

    SMCCC_SERVICE_TRUSTED_OS = 50,
    SMCCC_SERVICE_TRUSTED_OS_END = 63
};

struct arm_smccc_res {
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
};

void arm_smccc_smc(unsigned long a0, unsigned long a1, unsigned long a2,
                   unsigned long a3, unsigned long a4, unsigned long a5,
                   unsigned long a6, unsigned long a7,
                   struct arm_smccc_res *res);

void arm_smccc_hvc(unsigned long a0, unsigned long a1, unsigned long a2,
                   unsigned long a3, unsigned long a4, unsigned long a5,
                   unsigned long a6, unsigned long a7,
                   struct arm_smccc_res *res);

int handle_arm_smccc(struct core_regs *regs);

#endif
