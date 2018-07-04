#ifndef __SMCCC_H__
#define __SMCCC_H__

#include <arch/armv7.h>

#define SMCCC_TYPE_SHIFT    31
#define SMCCC_CONV_SHIFT    30
#define SMCCC_SERVICE_SHIFT 24
#define SMCCC_FN_NUM_SHIFT   0

#define SMCCC_TYPE_MASK     0x1
#define SMCCC_CONV_MASK     0x1
#define SMCCC_SERVICE_MASK  0x3f
#define SMCCC_FN_NUM_MASK   0xffff

#define __SMCCC_GET_FROM_ID(desc, id) \
    (((id) >> SMCCC_##desc##_SHIFT) & SMCCC_##desc##_MASK)

#define SMCCC_IS_FAST_CALL(fn)  __SMCCC_GET_FROM_ID(TYPE, fn)
#define SMCCC_IS_64(fn)         __SMCCC_GET_FROM_ID(CONV, fn)
#define SMCCC_SERVICE(fn)       __SMCCC_GET_FROM_ID(SERVICE, fn)
#define SMCCC_FN_NUM(fn)        __SMCCC_GET_FROM_ID(FN_NUM, fn)

#define SMCCC_FN_ID(type, conv, service, fn_num) ( \
    (((type) & SMCCC_CALL_TYPE_MASK) << SMCCC_CALL_TYPE_SHIFT) | \
    (((conv) & SMCCC_CONV_MASK) << SMCCC_CONV_SHIFT) | \
    (((service) & SMCCC_SERVICE_MASK) << SMCCC_SERVICE_SHIFT) | \
    (((fn_num) & SMCCC_FN_NUM_MASK) << SMCCC_FN_NUM_SHIFT))

enum smccc_call_type {
    SMCCC_TYPE_STD = 0,
    SMCCC_TYPE_FAST
};

enum smccc_convention {
    SMCCC_CONV_32 = 0,
    SMCCC_CONV_64
};

enum smccc_service_range {
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

#define INVALID_SMCCC_FN -1

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
