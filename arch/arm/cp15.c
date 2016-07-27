#include <stdio.h>
#include <arch/armv7.h>
#include <drivers/vdev/vdev_timer.h>

#define opc2_bit                (0x7 << 17)
#define opc1_bit                (0x7 << 14)
#define crn_bit                 (0xf << 10)
#define rt_bit                  (0xf <<  5)
#define crm_bit                 (0xf <<  1)

#define opc2(iss)               (((iss) & opc2_bit) >> 17)
#define opc1(iss)               (((iss) & opc1_bit) >> 14)
#define crn(iss)                (((iss) & crn_bit)  >> 10)
#define rt(iss)                 (((iss) & rt_bit)   >>  5)
#define crm(iss)                (((iss) & crm_bit)  >>  1)

#define decode_dir(iss)         (iss & 1)
#define decode_cp15(iss)        __CP32(15, crn(iss), opc1(iss), crm(iss), opc2(iss))

int32_t emulate_cp15_32(struct core_regs *regs, uint32_t iss)
{
    int32_t ret = 0;
    uint8_t rt = rt(iss);
    uint8_t dir = decode_dir(iss);

    switch (decode_cp15(iss)) {
        case CP32(MIDR):
            {
                if (!dir) {
                    write_cp32(regs->gpr[rt], MIDR);
                } else {
                    regs->gpr[rt] = read_cp32(MIDR);
                }
                ret = 0;
                break;
            }
        case CP32(CNTFRQ)    :
        case CP32(CNTKCTL)   :
        case CP32(CNTP_TVAL) :
        case CP32(CNTP_CTL)  :
        case CP32(CNTV_TVAL) :
        case CP32(CNTV_CTL)  :
            vdev_timer_access32(dir, decode_cp15(iss), &regs->gpr[rt]);
            break;
        default:
            printf("not implement: %x\n", decode_cp15(iss));
            ret = -1;
            break;
    }

    return ret;
}

