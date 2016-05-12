#include <vdev.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdio.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>

#define SYS_ID          (0x000)
#define SYS_SW          (0x004)
#define SYS_LED         (0x008)
#define SYS_100HZ       (0x024)
#define SYS_FLAGS       (0x030)
#define SYS_FLAGSSET    (0x030)
#define SYS_FLAGSCLR    (0x034)
#define SYS_NVFLAGS     (0x038)
#define SYS_NVFLAGSSET  (0x038)
#define SYS_NVFLAGSCLR  (0x03C)
#define SYS_MCI         (0x048)
#define SYS_FLASH       (0x04C)
#define SYS_CFGSW       (0x058)
#define SYS_24MHZ       (0x05C)
#define SYS_MISC        (0x060)
#define SYS_DMA         (0x064)
#define SYS_PROC_ID0    (0x084)
#define SYS_PROC_ID1    (0x088)
#define SYS_CFGDATA     (0x0A0)
#define SYS_CFGCTRL     (0x0A4)
#define SYS_CFGSTAT     (0x0A8)

#define SYSREG_BASE  0x1C010000

int32_t vsysreg_read_handler(void *pdata, uint32_t offset);
int32_t vsysreg_write_handler(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vsysreg_create_instance(void **pdata);

struct vdev_module vdev_sysreg = {
    .name   = "vsysreg",
    .base   = SYSREG_BASE,
    .size   = 0x1000,
    .read   = vsysreg_read_handler,
    .write  = vsysreg_write_handler,
    .create = vsysreg_create_instance,
};

struct vsysreg {
    uint32_t sys_id;
    uint32_t sys_sw;
    uint32_t sys_led;
    uint32_t sys_100hz;
    uint32_t sys_flags;
    uint32_t sys_flagsclr;
    uint32_t sys_nvflags;
    uint32_t sys_nvflagsset;
    uint32_t sys_nvflagsclr;
    uint32_t sys_cfgsw;
    uint32_t sys_24mhz;
    uint32_t sys_misc;
    uint32_t sys_pcie_cntl;
    uint32_t sys_pcie_gbe_l;
    uint32_t sys_pcie_gbe_h;
    uint32_t sys_proc_id0;
    uint32_t sys_proc_id1;
    uint32_t sys_speed;
};

#include <stdlib.h>
#include <string.h>
int32_t vsysreg_create_instance(void **pdata)
{
    struct vsysreg *vsysreg = malloc(sizeof(struct vsysreg));

    memset(vsysreg, 0, sizeof(struct vsysreg));
    *pdata = vsysreg;

    return 0;
}

extern uint32_t linux_smp_pen;
int32_t vsysreg_write_handler(void *pdata, uint32_t offset, uint32_t *addr)
{
    struct vsysreg *sysreg = pdata;

    switch (offset) {
    case SYS_ID:
        writel(readl(addr), SYSREG_BASE + SYS_ID);
        break;

    case SYS_SW:
        writel(readl(addr), SYSREG_BASE + SYS_SW);
        break;

    case SYS_LED:
        writel(readl(addr), SYSREG_BASE + SYS_LED);
        break;

    case SYS_FLAGSSET: {
        int vcpuid;
        struct vcpu *target_vcpu = NULL;
        struct vmcb *vm = get_current_vm();
        uint32_t pc = readl(addr);

        for (vcpuid = 1; vcpuid < vm->num_vcpus; vcpuid++) {
            target_vcpu = vm->vcpu[vcpuid];
            target_vcpu->regs.core_regs.pc = pc;
            printf("vcpu[%d] attatched to pcpu[%d] \n", target_vcpu->vcpuid, target_vcpu->pcpuid);
            sched_vcpu_attach(target_vcpu->vcpuid, target_vcpu->pcpuid);
            target_vcpu->state = VCPU_ACTIVATED;
        }

        sysreg->sys_flags = pc;
        break;
    }

    case SYS_FLAGSCLR:
        writel(readl(addr), SYSREG_BASE + SYS_FLAGSCLR);
        break;

    case SYS_NVFLAGSSET:
        writel(readl(addr), SYSREG_BASE + SYS_NVFLAGSSET);
        break;

    case SYS_NVFLAGSCLR:
        writel(readl(addr), SYSREG_BASE + SYS_NVFLAGSCLR);
        break;

    case SYS_FLASH:
        writel(readl(addr), SYSREG_BASE + SYS_FLASH);
        break;

    case SYS_CFGSW:
        writel(readl(addr), SYSREG_BASE + SYS_CFGSW);
        break;

    case SYS_MISC:
        writel(readl(addr), SYSREG_BASE + SYS_MISC);
        break;

    case SYS_DMA:
        writel(readl(addr), SYSREG_BASE + SYS_DMA);
        break;

    case SYS_PROC_ID0:
        writel(readl(addr), SYSREG_BASE + SYS_PROC_ID0);
        break;

    case SYS_PROC_ID1:
        writel(readl(addr), SYSREG_BASE + SYS_PROC_ID1);
        break;

    case SYS_CFGDATA:
        writel(readl(addr), SYSREG_BASE + SYS_CFGDATA);
        break;

    case SYS_CFGCTRL:
        writel(readl(addr), SYSREG_BASE + SYS_CFGCTRL);
        break;

    case SYS_CFGSTAT:
        writel(readl(addr), SYSREG_BASE + SYS_CFGSTAT);
        break;


    default:
        printf("regsys write fail : %x\n", SYSREG_BASE + offset);
        writel(readl(addr), SYSREG_BASE + offset);
        break;
    }

    return 0;
}

int32_t vsysreg_read_handler(void *pdata, uint32_t offset)
{
    struct vsysreg *sysreg = pdata;

    switch (offset) {
    case SYS_ID:
        return readl(SYSREG_BASE + SYS_ID);

    case SYS_SW:
        return readl(SYSREG_BASE + SYS_SW);

    case SYS_LED:
        return readl(SYSREG_BASE + SYS_LED);

    case SYS_100HZ:
        return readl(SYSREG_BASE + SYS_100HZ);

    case SYS_FLAGS:
        return sysreg->sys_flags;

    case SYS_NVFLAGS:
        return readl(SYSREG_BASE + SYS_NVFLAGS);

    case SYS_FLASH:
        return readl(SYSREG_BASE + SYS_FLASH);

    case SYS_MCI:
        return readl(SYSREG_BASE + SYS_MCI);

    case SYS_CFGSW:
        return readl(SYSREG_BASE + SYS_CFGSW);

    case SYS_24MHZ:
        return readl(SYSREG_BASE + SYS_24MHZ);

    case SYS_MISC:
        return readl(SYSREG_BASE + SYS_MISC);

    case SYS_DMA:
        return readl(SYSREG_BASE + SYS_DMA);

    case SYS_PROC_ID0:
        return readl(SYSREG_BASE + SYS_PROC_ID0);

    case SYS_PROC_ID1:
        return readl(SYSREG_BASE + SYS_PROC_ID1);

    case SYS_CFGDATA:
        return readl(SYSREG_BASE + SYS_CFGDATA);

    case SYS_CFGCTRL:
        return readl(SYSREG_BASE + SYS_CFGCTRL);

    case SYS_CFGSTAT:
        return readl(SYSREG_BASE + SYS_CFGSTAT);

    default:
        printf("regsys read fail : %x\n", SYSREG_BASE + offset);
        return readl(SYSREG_BASE + offset);
        break;
    }

    return 0;
}

hvmm_status_t vdev_sysreg_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    vdev_register(&vdev_sysreg);

    printf("vdev registered:'%s'\n", vdev_sysreg.name);

    return result;
}

vdev_module_init(vdev_sysreg_init);

