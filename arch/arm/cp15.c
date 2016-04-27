#include <stdio.h>
#include <arch/armv7.h>
#include "traps.h"

#define __CP32(cp, n, opc1, m, opc2)    (n << 12 | opc1 << 8 | m << 4 | opc2 << 0)
#define CP32(args...)                   __CP32(args)

#define opc2_bit                (0x7 << 17)
#define opc1_bit                (0x7 << 14)
#define crn_bit                 (0xf << 10)
#define crm_bit                 (0xf <<  1)

#define opc2(iss)               ((iss & opc2_bit) >> 17)
#define opc1(iss)               ((iss & opc1_bit) >> 14)
#define crn(iss)                ((iss & crn_bit)  >> 10)
#define crm(iss)                ((iss & crm_bit)  >>  1)

#define decode_cp15(iss)        crn(iss) << 12 | opc1(iss) << 8 | \
                                crm(iss) << 4  | opc2(iss)

#define decode_dir(iss)         (iss & 1)
#define decode_rt(iss)          ((iss & 0x1E0) >> 5)

int32_t emulate_cp15(struct core_regs *regs, uint32_t iss)
{
    int32_t ret = -1;
    uint8_t dir = decode_dir(iss);
    uint8_t rt = decode_rt(iss);

    switch (decode_cp15(iss)) {
    case CP32(MIDR): {
        if (!dir) {
            write_cp32(regs->gpr[rt], MIDR);
        } else {
            regs->gpr[rt] = read_cp32(MIDR);
        }
        ret = 0;
        break;
    }
    default:
        printf("not implement: %x\n", decode_cp15(iss));
        break;
    }

    return ret;
}
