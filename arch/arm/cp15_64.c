#include <stdio.h>
#include <arch/armv7.h>
#include <drivers/vdev/vdev_timer.h>

#define opc1_bit            (0xf << 16)
#define rt2_bit             (0xf << 10)
#define rt_bit              (0xf <<  5)
#define crm_bit             (0xf <<  1)

#define opc1(iss)           (((iss) & opc1_bit) >> 16)
#define rt2(iss)            (((iss) & rt2_bit)  >> 10)
#define rt(iss)             (((iss) & rt_bit)   >>  5)
#define crm(iss)            (((iss) & crm_bit)  >>  1)

#define decode_dir(iss)     ((iss) & 1)
#define decode_cp15(iss)    __CP64(15, opc1(iss), crm(iss))

int32_t emulate_cp15_64(struct core_regs *regs, uint32_t iss)
{
    int32_t ret = -1;
    uint8_t rt = rt(iss);
    uint8_t rt2 = rt2(iss);
    uint8_t dir = decode_dir(iss);

    switch (decode_cp15(iss)) {
    case CP64(CNTPCT)    :
    case CP64(CNTVCT)    :
    case CP64(CNTP_CVAL) :
    case CP64(CNTV_CVAL) :
        vdev_timer_access64(dir, decode_cp15(iss), &regs->gpr[rt], &regs->gpr[rt2]);
        break;
    default:
        printf("not implement: %x\n", decode_cp15(iss));
        break;
    }

    return ret;
}

