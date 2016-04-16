#include <stdio.h>
#include <vdev.h>
#include <drivers/sp804.h>
#include <io.h>

int32_t vtimer_read(void *pdata, uint32_t offset);
int32_t vtimer_write(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vtimer_create(void **pdata);

struct sp804 {
    uint32_t timer1_load;
    uint32_t timer1_value;
    uint32_t timer1_control;
    uint32_t timer1_intclr;
    uint32_t timer1_ris;
    uint32_t timer1_mis;
    uint32_t timer1_bgload;

    uint32_t timer2_load;
    uint32_t timer2_value;
    uint32_t timer2_control;
    uint32_t timer2_intclr;
    uint32_t timer2_ris;
    uint32_t timer2_mis;
    uint32_t timer2_bgload;

    uint32_t timer_itcr;
    uint32_t timer_itop;

    uint32_t timer_peiphid0;
    uint32_t timer_peiphid1;
    uint32_t timer_peiphid2;
    uint32_t timer_peiphid3;

    uint32_t timer_pcellid0;
    uint32_t timer_pcellid1;
    uint32_t timer_pcellid2;
    uint32_t timer_pcellid3;

};

struct vdev_module sp804_vtimer1 = {
    .name = "vdev_sp804",
    .base = TIMER1_BASE,
    .size = 0x1000,
    .read = vtimer_read,
    .write = vtimer_write,
    .create = vtimer_create,
};

#include <stdlib.h>
int32_t vtimer_create(void **pdata)
{
    struct sp804 *vtimer = malloc(sizeof(struct sp804));

    *pdata = vtimer;
    return 0;
}

int32_t vtimer_write(void *pdata, uint32_t offset, uint32_t *addr)
{
    struct sp804 *sp804 = pdata;

    switch (offset) {
    case TIMER1_LOAD(0):
        sp804->timer1_load = readl(addr);
        writel(sp804->timer1_load, TIMER1_LOAD(TIMER1_BASE));
        break;

    case TIMER1_CONTROL(0):
        sp804->timer1_control = readl(addr);
        writel(sp804->timer1_control, TIMER1_CONTROL(TIMER1_BASE));
        break;

    case TIMER1_INTCLR(0):
        sp804->timer1_intclr = readl(addr);
        writel(sp804->timer1_intclr, TIMER1_INTCLR(TIMER1_BASE));
        break;

    case TIMER1_BGLOAD(0):
        //printf("TIMER1_BGLOAD: %x\n", readl(addr));
        sp804->timer1_bgload = readl(addr);
        writel(sp804->timer1_bgload, TIMER1_BGLOAD(TIMER1_BASE));
        break;

    case TIMER2_LOAD(0):
        printf("TIMER2_LOAD %x\n", readl(addr));
        sp804->timer2_load = readl(addr);
        writel(sp804->timer2_load, TIMER2_LOAD(TIMER1_BASE));
        break;

    case TIMER2_CONTROL(0):
        printf("TIMER2_CONTROL %x\n", readl(addr));
        sp804->timer2_control = readl(addr);
        writel(sp804->timer2_control, TIMER2_CONTROL(TIMER1_BASE));
        break;

    case TIMER2_INTCLR(0):
        printf("TIMER2_INTCLR %x\n", readl(addr));
        sp804->timer2_intclr = readl(addr);
        writel(sp804->timer2_intclr, TIMER2_INTCLR(TIMER1_BASE));
        break;

    case TIMER2_BGLOAD(0):
        //printf("[write] offset: %x, %x\n", offset, readl(addr));
        sp804->timer2_bgload = readl(addr);
        writel(sp804->timer2_bgload, TIMER2_BGLOAD(TIMER1_BASE));
        break;

    case TIMER_ITCR(0):
        //printf("[write] offset: %x, %x\n", offset, readl(addr));
        sp804->timer_itcr = readl(addr);
        writel(sp804->timer_itcr, TIMER_ITCR(TIMER1_BASE));
        break;

    case TIMER_ITOP(0):
        //printf("[write] offset: %x, %x\n", offset, readl(addr));
        sp804->timer_itop = readl(addr);
        writel(sp804->timer_itop, TIMER_ITOP(TIMER1_BASE));
        break;

    default:
        //printf("%s ERROR!! [write] offset: %x, %x\n", __func__, offset, readl(addr));
        break;
    }
    return 0;
}

int32_t vtimer_read(void *pdata, uint32_t offset)
{
    struct sp804 *sp804 = pdata;

    switch (offset) {
    case TIMER1_LOAD(0):
        return sp804->timer1_load;
        break;

    case TIMER1_VALUE(0):
        sp804->timer1_value = readl(TIMER1_VALUE(TIMER1_BASE));
        return sp804->timer1_value;
        break;

    case TIMER1_CONTROL(0):
        return sp804->timer1_control;
        break;

    case TIMER1_RIS(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer1_ris;
        break;

    case TIMER1_MIS(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer1_mis;
        break;

    case TIMER1_BGLOAD(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer1_bgload;
        break;

    case TIMER2_LOAD(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_load;
        break;

    case TIMER2_VALUE(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_value;
        break;

    case TIMER2_CONTROL(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_control;
        break;

    case TIMER2_RIS(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_ris;
        break;

    case TIMER2_MIS(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_mis;
        break;

    case TIMER2_BGLOAD(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer2_bgload;
        break;

    case TIMER_ITCR(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_itcr;
        break;

    case TIMER_RERIPHID0(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_peiphid0;
        break;

    case TIMER_RERIPHID1(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_peiphid1;
        break;

    case TIMER_RERIPHID2(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_peiphid2;
        break;

    case TIMER_RERIPHID3(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_peiphid3;
        break;

    case TIMER_RPCELL0(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_pcellid0;
        break;

    case TIMER_RPCELL1(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_pcellid1;
        break;

    case TIMER_RPCELL2(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_pcellid2;
        break;

    case TIMER_RPCELL3(0):
        //printf("[read] offset: %x, %x\n", offset);
        return sp804->timer_pcellid3;
        break;

    default:
        //printf("%s ERROR!! [read] offset: %x, %x\n", __func__, offset);
        break;
    }
    return 0;
}

hvmm_status_t vdev_sp804_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    vdev_register(&sp804_vtimer1);

    printf("vdev registered:'%s'\n", sp804_vtimer1.name);

    return result;
}

vdev_module_init(vdev_sp804_init);

