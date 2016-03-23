#include <arch/gic_regs.h>
#include <vdev.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdio.h>
#include <core/vm/virq.h>
#include <core/vm.h>
#include <core/vm/vcpu.h>
#include "../../drivers/gic-v2.h"
#include <core/vm/vgic.h>

#define READ    0
#define WRITE   1

/* return the bit position of the first bit set from msb
 * for example, firstbit32(0x7F = 111 1111) returns 7
 */
#define firstbit32(word) (31 - asm_clz(word))

static uint32_t ITLinesNumber = 0;
static struct vdev_memory_map _vdev_gicd_info = {
    .base = CFG_GIC_BASE_PA | GICD_OFFSET,
    .size = 4096,
};

static void set_enable(struct vcpu* vcpu, uint32_t current_status, uint8_t n, uint32_t old_status)
{
    uint32_t delta = old_status ^ current_status;

    while (delta) {
        uint32_t offset = firstbit32(delta);
        uint32_t virq = offset + (32 * n);
        uint32_t pirq = virq_to_pirq(&vcpu->virq, virq);

        pirq = (pirq == PIRQ_INVALID ? virq : pirq);

        virq_enable(&vcpu->virq, pirq, virq);
        pirq_enable(&vcpu->virq, pirq, virq);
        gic_enable_irq(pirq);

        delta &= ~(1 << offset);
    }
}

static void set_clear(struct vcpu *vcpu, uint32_t current_status, uint8_t n, uint32_t old_status)
{
    uint32_t delta = old_status ^ current_status;

    while (delta) {

        uint32_t offset = firstbit32(delta);
        uint32_t virq = offset + (32 * n);
        uint32_t pirq = virq_to_pirq(&vcpu->virq, virq);

        virq_disable(&vcpu->virq, virq);
        pirq_disable(&vcpu->virq, pirq);
        // TODO(casionwoo) : When VM try to interrupt clear, must be checked every VM clear the interrupt. Then clear the irq
        // gic_disable_irq(pirq);

        delta &= ~(1 << offset);
    }
}

static hvmm_status_t handler_SGIR(struct vcpu *vcpu, uint32_t offset, uint32_t value)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    struct gicd_banked *gicd_banked;

    uint32_t target_cpu_interfaces = 0;
    uint32_t sgi_id = value & GICD_SGIR_SGI_INT_ID_MASK;
    uint8_t target_vcpuid;

    switch (value & GICD_SGIR_TARGET_LIST_FILTER_MASK) {
        case GICD_SGIR_TARGET_LIST:
            target_cpu_interfaces = ((value & GICD_SGIR_CPU_TARGET_LIST_MASK) >> GICD_SGIR_CPU_TARGET_LIST_OFFSET);
            break;

        case GICD_SGIR_TARGET_OTHER:
            target_cpu_interfaces = ~(0x1 << vcpu->vmid);
            break;

        case GICD_SGIR_TARGET_SELF:
            target_cpu_interfaces = (0x1 << vcpu->vmid);
            break;

        default:
            return result;
    }

    // FIXME(casionwoo) : This part should have some policy for interprocessor communication
    while (target_cpu_interfaces) {
        uint8_t target_cpu_interface = target_cpu_interfaces & 0x01;

        if (target_cpu_interface && (vcpu = vcpu_find(target_vcpuid))) {
            uint32_t n = sgi_id >> 2;
            uint32_t reg_offset = sgi_id % 4;

            gicd_banked = &vcpu->virq.gicd_banked;
            gicd_banked->SPENDSGIR[n] = 0x1 << ((reg_offset * 8) + target_cpu_interface);
            result = virq_inject(target_vcpuid, sgi_id, sgi_id, SW_IRQ);
        }

        target_vcpuid++;
        target_cpu_interfaces = target_cpu_interfaces >> 1;
    }

    return result;
}

static int32_t vdev_gicd_write_handler(struct arch_vdev_trigger_info *info)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;
    struct vcpu *vcpu = get_current_vcpu();
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd *gicd = &vm->vgic.gicd;
    struct gicd_banked *gicd_banked = &vcpu->virq.gicd_banked;

    uint32_t old_status;

    switch (offset)
    {
        case GICD_CTLR:
            gicd->CTLR = readl(info->raw);
            return HVMM_STATUS_SUCCESS;

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
        {
            uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

            if (n == 0) {
                gicd_banked->IGROUPR = readl(info->raw);
            } else if ((n > 0) && (n < (ITLinesNumber + 1))) {
                gicd->IGROUPR[n] = readl(info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
        {
            uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

            if (n == 0) {
                old_status = gicd_banked->ISENABLER;
                gicd_banked->ISENABLER |= readl(info->raw);
                set_enable(vcpu, gicd_banked->ISENABLER, n, old_status);
            } else {
                old_status = gicd->ISENABLER[n];
                gicd->ISENABLER[n] |= readl(info->raw);
                set_enable(vcpu, gicd->ISENABLER[n], n, old_status);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
        {
            uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

            if (n == 0) {
                old_status = gicd_banked->ICENABLER;
                gicd_banked->ICENABLER |= readl(info->raw);
                set_clear(vcpu, gicd_banked->ICENABLER, n, old_status);
            } else {
                old_status = gicd->ICENABLER[n];
                gicd->ICENABLER[n] |= readl(info->raw);
                set_clear(vcpu, gicd->ICENABLER[n], n, old_status);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
        {
            uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

            if (n == 0) {
                gicd_banked->ISPENDR |= readl(info->raw);
            } else {
                gicd->ISPENDR[n] |= readl(info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
        {
            uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

            if (n == 0) {
                gicd_banked->ICPENDR = ~(readl(info->raw));

            } else {
                gicd->ICPENDR[n] = ~(readl(info->raw));
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
        {
            uint32_t n = (offset - GICD_ISACTIVER(0)) >> 2;

            // Overwrite has no effect.
            GICD_WRITE(GICD_ISACTIVER(n), readl(info->raw));

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
        {
            uint32_t n = (offset - GICD_ICACTIVER(0)) >> 2;

            GICD_WRITE(GICD_ICACTIVER(n), readl(info->raw));

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
        {
            uint32_t n = ((offset - GICD_IPRIORITYR(0)) >> 2);

            if (n < NR_BANKED_IPRIORITYR) {
                gicd_banked->IPRIORITYR[n] = readl(info->raw);
            } else {
                gicd->IPRIORITYR[n] = readl(info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }


        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
        {
            uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;

            if (n < NR_BANKED_ITARGETSR) {
                gicd_banked->ITARGETSR[n] |= readl(info->raw);
            } else {
                gicd->ITARGETSR[n] |= readl(info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
        {
            uint32_t n = (offset - GICD_ICFGR(0));

            if (n == 1) {
                gicd_banked->ICFGR = readl(info->raw);
            } else {
                gicd->ICFGR[n] = readl(info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            printf("vgicd: GICD_NSACR write not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_SGIR:
            return handler_SGIR(vcpu, offset, readl(info->raw));

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
        {
            uint32_t n = offset - GICD_CPENDSGIR(0);

            gicd_banked->CPENDSGIR[n] = ~(readl(info->raw));

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
        {
            uint32_t n = offset - GICD_SPENDSGIR(0);

            gicd_banked->SPENDSGIR[n] = ~(readl(info->raw));

            return HVMM_STATUS_SUCCESS;
        }

        case 0xD00 ... 0xDFC:
            printf("vgicd: GICD_PPISPISR_CA15 write not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        default:
            printf("there's no corresponding address or read only register\n");
            return HVMM_STATUS_BAD_ACCESS;
    }

    return HVMM_STATUS_SUCCESS;
}

static int32_t vdev_gicd_read_handler(struct arch_vdev_trigger_info *info)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;
    struct vcpu *vcpu = get_current_vcpu();
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd *gicd = &vm->vgic.gicd;
    struct gicd_banked *gicd_banked = &vcpu->virq.gicd_banked;

    switch (offset)
    {
        case GICD_CTLR:
            writel(gicd->CTLR, info->raw);
            return HVMM_STATUS_SUCCESS;

        case GICD_TYPER:
            writel(gicd->TYPER, info->raw);
            return HVMM_STATUS_SUCCESS;

        case GICD_IIDR:
            writel(gicd->IIDR, info->raw);
            return HVMM_STATUS_SUCCESS;

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
        {
            uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

            if (n == 0) {
                writel(gicd_banked->IGROUPR, info->raw);
            }
            else if ((n > 0) && (n < (ITLinesNumber + 1))) {
                writel(gicd->IGROUPR[n], info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
        {
            uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

            if (n == 0) {
                writel(gicd_banked->ISENABLER, info->raw);
            } else {
                writel(gicd->ISENABLER[n], info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
        {
            uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

            if (n == 0) {
                writel(gicd_banked->ICENABLER, info->raw);
            } else {
                writel(gicd->ICENABLER[n], info->raw);
            }
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
        {
            uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

            if (n == 0) {
                writel(gicd_banked->ISPENDR, info->raw);
            } else {
                writel(gicd->ISPENDR[n], info->raw);
            }
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
        {
            uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

            if (n == 0) {
                writel(gicd_banked->ICPENDR, info->raw);
            } else {
                writel(gicd->ICPENDR[n], info->raw);
            }
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
        {
            uint32_t n = (offset - GICD_ISACTIVER(0)) >> 2;

            writel(GICD_READ(GICD_ISACTIVER(n)), info->raw);


            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
        {
            uint32_t n = (offset - GICD_ICACTIVER(0)) >> 2;

            writel(GICD_READ(GICD_ICACTIVER(n)), info->raw);

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
        {
            uint32_t n = ((offset - GICD_IPRIORITYR(0)) >> 2);

            if (n < NR_BANKED_IPRIORITYR) {
                writel(gicd_banked->IPRIORITYR[n], info->raw);
            } else {
                writel(gicd->IPRIORITYR[n], info->raw);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
        {
            uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;

            if (n < NR_BANKED_ITARGETSR) {
                writel(gicd_banked->ITARGETSR[n], info->raw);
            } else {
                writel(gicd->ITARGETSR[n], info->raw);
            }
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
        {
            uint32_t n = (offset - GICD_ICFGR(0));

            if (n == 1) {
                writel(gicd_banked->ICFGR, info->raw);
            } else {
                writel(gicd->ICFGR[n], info->raw);
            }
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            printf("vgicd: GICD_NSACR read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_SGIR:
            printf("GICD_SGIR is WO\n");
            return HVMM_STATUS_SUCCESS;

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
        {
            writel(gicd_banked->CPENDSGIR[offset - GICD_CPENDSGIR(0)], info->raw);
            return HVMM_STATUS_SUCCESS;
        }

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
        {
            writel(gicd_banked->SPENDSGIR[offset - GICD_SPENDSGIR(0)], info->raw);
            return HVMM_STATUS_SUCCESS;
        }

        case 0xD00 ... 0xDFC:
            printf("vgicd: GICD_PPISPISR_CA15 read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        default:
            printf("there's no corresponding address\n");
            return HVMM_STATUS_BAD_ACCESS;
    }

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t vdev_gicd_post(struct core_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) { /* Thumb */
        isize = 2;
    }

    regs->pc += isize;

    return 0;
}

static int32_t vdev_gicd_check(struct arch_vdev_trigger_info *info)
{
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    if (info->fipa >= _vdev_gicd_info.base
            && offset < _vdev_gicd_info.size) {
        return 0;
    }
    return VDEV_NOT_FOUND;
}

static hvmm_status_t vdev_gicd_reset_values(void)
{
    ITLinesNumber = GICv2.ITLinesNumber;
    return HVMM_STATUS_SUCCESS;
}

struct vdev_ops _vdev_gicd_ops = {
    .init = vdev_gicd_reset_values,
    .check = vdev_gicd_check,
    .read = vdev_gicd_read_handler,
    .write = vdev_gicd_write_handler,
    .post = vdev_gicd_post
};

struct vdev_module _vdev_gicd_module = {
    .name = "K-Hypervisor vDevice GICD Module",
    .author = "Kookmin Univ.",
    .ops = &_vdev_gicd_ops
};

hvmm_status_t vdev_gicd_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(VDEV_LEVEL_LOW, &_vdev_gicd_module);
    if (result == HVMM_STATUS_SUCCESS) {
        printf("vdev registered:'%s'\n", _vdev_gicd_module.name);
    } else {
        printf("%s: Unable to register vdev:'%s' code=%x\n", __func__,
                    _vdev_gicd_module.name, result);
    }

    return result;
}

vdev_module_low_init(vdev_gicd_init);

