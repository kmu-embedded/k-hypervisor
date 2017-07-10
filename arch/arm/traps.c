#include <stdio.h>
#include <arch/armv7.h>

#include "cp15.h"

// TODO(wonseok): If the traps cause the undefined exception or
//                abort exception, we must forward the exception to guest VM.
#define INVALID_HSR         -1

int do_hyp_trap(struct core_regs *regs)
{
    uint8_t pcpuid = smp_processor_id();
    int ret = INVALID_HSR;
    uint32_t hsr = read_cp32(HSR);
    uint32_t ec = EC(hsr);
    uint32_t il = IL(hsr);
    uint32_t iss = ISS(hsr);

    switch (ec) {
    case EC_UNK:
        /*
         * if (HCR.TGE == 1) 
         *      handling undefined instruction
         * else 
         *      invalid_hsr;
         */
    case EC_WFI_WFE:
        /* if (CV(iss) == 1) {
         *     condition = COND(ISS); 
         * }
         */
        break;
    case EC_MCR_MRC_CP15:
        ret = emulate_cp15_32(regs, iss);
        break;
    case EC_MCRR_MRRC_CP15:
        ret = emulate_cp15_64(regs, iss);
        break;
    case EC_MCR_MRC_CP14:
    case EC_LDC_STC_CP14:
    case EC_HCRTR_CP0_CP13:
    case EC_MRC_VMRS_CP10:
    case EC_BXJ:
    case EC_MRRC_CP14:
    case EC_SVC:
    case EC_HVC:
    case EC_SMC:
    case EC_PABT_FROM_GUEST:
    case EC_PABT_FROM_HYP:
        break;
    case EC_DABT_FROM_GUEST:
        ret = handle_data_abort(regs, iss);
    case EC_DABT_FROM_HYP:
        break;
    default:
        break;
    }

    if (ret == INVALID_HSR) {
        goto trap_error;
    }

    if (il == IL_ARM) {
        regs->pc += 4;
    } else {
        regs->pc += 2;
    }

    return 0;

trap_error:
    printf("CPU[%d] %s EC: 0x%x ISS: 0x%x\n", pcpuid, __func__, ec, iss);
    printf("r0 %x\n", regs->gpr[0]);
    printf("r1 %x\n", regs->gpr[1]);
    printf("r2 %x\n", regs->gpr[2]);
    printf("r3 %x\n", regs->gpr[3]);
    printf("guest pc is %x\n", regs->pc);
    printf("hifar %x\n", read_cp32(HIFAR));
    while (1) ;

    return INVALID_HSR;
}
