#include <stdio.h>
#include <arch/armv7.h>
#include "traps.h"
#include "cp15.h"

#define decode_ec(hsr)          (hsr >> 26)
#define decode_il(hsr)          (hsr & (1 << 25))
#define decode_iss(hsr)         (hsr & ~(0xfe000000))

// TODO(wonseok): If the traps cause the undefined exception or
//                abort exception, we must forward the exception to guest VM.
int do_hyp_trap(struct core_regs *regs)
{
    uint8_t pcpuid = smp_processor_id();
    int ret = -1;
    uint32_t hsr = read_cp32(HSR);
    uint32_t ec = decode_ec(hsr);
    uint32_t il = decode_il(hsr);
    uint32_t iss = decode_iss(hsr);
    uint32_t hifar = read_cp32(HIFAR);
    uint32_t hpfar = read_cp32(HPFAR);
    uint32_t hdfar = read_cp32(HDFAR);
    uint32_t dfsr = read_cp32(DFSR);

    switch (ec) {
    case WFI_WFE:
        break;
    case MCR_MRC_CP15:
        ret = emulate_cp15(regs, iss);
        break;
    case MCRR_MRRC_CP15:
    case HCRTR_CP0_CP13:
    case MRC_VMRS_CP10:
    case HVC:
        break;
    case DABT_FROM_GUEST:
        ret = handle_data_abort(regs, iss);
        break;
    default:
        break;
    }

    if (ret == -1) {
        goto trap_error;
    }

    if (il) {
        regs->pc += 4;
    } else {
        regs->pc += 2;
    }

    return 0;

trap_error:
    printf("CPU[%d] %s EC: 0x%x ISS: 0x%x\n", pcpuid, __func__, ec, iss);
    printf("HSR : %x\n", hsr);
    printf("HPFAR : %x\n", hpfar);
    printf("HIFAR : %x\n", hifar);
    printf("HDFAR : %x\n", hdfar);
    printf("DFSR : %x\n", dfsr);
    printf("r0 %x\n", regs->gpr[0]);
    printf("r1 %x\n", regs->gpr[1]);
    printf("r2 %x\n", regs->gpr[2]);
    printf("r3 %x\n", regs->gpr[3]);
    printf("guest pc is %x\n", regs->pc);
    while (1) ;

    return -1;
}
