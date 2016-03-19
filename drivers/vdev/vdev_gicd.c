#include <arch/gic_regs.h>
#include <vdev.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdio.h>
#include "vdev_gicd.h"
#include <core/vm/virq.h>
#include <core/vm.h>
#include <core/vm/vcpu.h>
#include "../../drivers/gic-v2.h"

#define READ    0
#define WRITE   1

/* return the bit position of the first bit set from msb
 * for example, firstbit32(0x7F = 111 1111) returns 7
 */
#define firstbit32(word) (31 - asm_clz(word))

static hvmm_status_t handler_000(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ISCENABLER(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ISCPENDR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ISCACTIVER(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_PPISPISR_CA15(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_NSACR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);

static struct vdev_memory_map _vdev_gicd_info = { .base =
        CFG_GIC_BASE_PA | GICD_OFFSET, .size = 4096,
};

static hvmm_status_t handler_000(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    /* CTLR;              0x000 RW*/
    /* TYPER;             RO*/
    /* IIDR;              RO*/
    /* IGROUPR[32];       0x080 ~ 0x0FF */
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);

    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t woffset = offset / 4;

    switch (woffset) {
    case __GICD_CTLR: /* RW */
        if (write) {
            regs->CTLR = *pvalue;
        } else {
            *pvalue = regs->CTLR;
        }
        result = HVMM_STATUS_SUCCESS;
        break;
    case __GICD_TYPER: /* RO */
        if (write == 0) {
            *pvalue = regs->TYPER;
            result = HVMM_STATUS_SUCCESS;
        }
        break;
    case __GICD_IIDR: /* RO */
        if (write == 0) {
            *pvalue = regs->IIDR;
            result = HVMM_STATUS_SUCCESS;
        }
        break;
    default: { /* RW GICD_IGROUPR */
        int igroup = woffset - __GICD_IGROUPR;
        if ((igroup == 0) && (igroup < VGICD_NUM_IGROUPR)) {
            if (write) {
                regs_banked->IGROUPR = *pvalue;
            } else {
                *pvalue = regs_banked->IGROUPR;
            }

            result = HVMM_STATUS_SUCCESS;
        } else if ((igroup > 0) && (igroup < VGICD_NUM_IGROUPR)) {
            if (write) {
                regs->IGROUPR[igroup] = *pvalue;
            } else {
                *pvalue = regs->IGROUPR[igroup];
            }

            result = HVMM_STATUS_SUCCESS;
        }
    }
    break;
    }
    if (result != HVMM_STATUS_SUCCESS)
        printf("vgicd: invalid access offset:%x write:%d\n", offset,
               write);

    return result;
}

static void vgicd_changed_istatus(vcpuid_t vcpuid, uint32_t istatus, uint8_t word_offset, uint32_t old_status)
{
    struct vcpu *vcpu = vcpu_find(vcpuid);
    uint32_t cstatus; /* changed bits only */
    uint32_t minirq;
    int bit;
    minirq = word_offset * 32;
    cstatus = old_status ^ istatus;

    while (cstatus) {
        uint32_t virq;
        uint32_t pirq;
        bit = firstbit32(cstatus);
        virq = minirq + bit;
        pirq = virq_to_pirq(&vcpu->virq, virq);
        if (pirq != PIRQ_INVALID) {
            /* changed bit */
            if (istatus & (1 << bit)) {
                printf("[%s : %d] enabled irq num is %d\n", __func__, __LINE__, bit + minirq);
                gic_configure_irq(pirq, IRQ_LEVEL_TRIGGERED);
                printf("vcpuid %d\tpirq %d\n", vcpuid, pirq);
                virq_enable(&vcpu->virq, pirq);
                gic_enable_irq(pirq);
            } else {
                printf("[%s : %d] disabled irq num is %d\n", __func__, __LINE__, bit + minirq);
                gic_disable_irq(pirq);
            }
        } else {
            printf("WARNING: Ignoring virq %d for guest %d has no mapped pirq\n", virq, vcpuid);
        }
        cstatus &= ~(1 << bit);
    }
}

static hvmm_status_t handler_ISCENABLER(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t *preg_s;
    uint32_t *preg_c;
    uint32_t old_status;

    if (write && *pvalue == 0) {
        /* Writes 0 -> Has no effect. */
        result = HVMM_STATUS_SUCCESS;
        return result;
    }
    if (((offset >> 2) - __GICD_ISENABLER) == 0 ||
            ((offset >> 2) - __GICD_ICENABLER) == 0) {
        preg_s = &(regs_banked->ISCENABLER);
        preg_c = &(regs_banked->ISCENABLER);
    } else {
        preg_s = &(regs->ISCENABLER[(offset >> 2) - __GICD_ISENABLER]);
        preg_c = &(regs->ISCENABLER[(offset >> 2) - __GICD_ICENABLER]);
    }
    if (access_size == VDEV_ACCESS_WORD) {
        if ((offset >> 2) < (__GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            /* ISENABLER */
            if (write) {
                old_status = *preg_s;
                *preg_s |= *pvalue;
                vgicd_changed_istatus(vcpuid, *preg_s, (offset >> 2) - __GICD_ISENABLER, old_status);
            } else {
                *pvalue = *preg_s;
            }

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= __GICD_ICENABLER
                   && (offset >> 2)
                   < (__GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            /* ICENABLER */
            if (write) {
                old_status = *preg_c;
                *preg_c &= ~(*pvalue);
                vgicd_changed_istatus(vcpuid, *preg_c, (offset >> 2) - __GICD_ICENABLER, old_status);
            } else {
                *pvalue = *preg_c;
            }

            result = HVMM_STATUS_SUCCESS;
        }
    } else if (access_size == VDEV_ACCESS_HWORD) {
        if ((offset >> 2) < (__GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            uint16_t *preg_s16 = (uint16_t *) preg_s;
            preg_s16 += (offset & 0x3) >> 1;
            if (write) {
                old_status = *preg_s;
                *preg_s16 |= (uint16_t) (*pvalue & 0xFFFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset >> 2) - __GICD_ISENABLER, old_status);
            } else {
                *pvalue = (uint32_t) * preg_s16;
            }

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= __GICD_ICENABLER
                   && (offset >> 2)
                   < (__GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            uint16_t *preg_c16 = (uint16_t *) preg_c;
            preg_c16 += (offset & 0x3) >> 1;
            if (write) {
                old_status = *preg_s;
                *preg_c16 &= ~((uint16_t) (*pvalue & 0xFFFF));
                vgicd_changed_istatus(vcpuid, *preg_c, (offset >> 2) - __GICD_ICENABLER, old_status);
            } else {
                *pvalue = (uint32_t) * preg_c16;
            }

            result = HVMM_STATUS_SUCCESS;
        }
    } else if (access_size == VDEV_ACCESS_BYTE) {
        if ((offset >> 2) < (__GICD_ISENABLER + VGICD_NUM_IENABLER)) {
            uint8_t *preg_s8 = (uint8_t *) preg_s;
            preg_s8 += (offset & 0x3);
            if (write) {
                old_status = *preg_s;
                *preg_s8 |= (uint8_t) (*pvalue & 0xFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset >> 2) - __GICD_ISENABLER, old_status);
            } else {
                *pvalue = (uint32_t) * preg_s8;
            }

            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= __GICD_ICENABLER
                   && (offset >> 2)
                   < (__GICD_ICENABLER + VGICD_NUM_IENABLER)) {
            uint8_t *preg_c8 = (uint8_t *) preg_c;
            preg_c8 += (offset & 0x3);
            if (write) {
                old_status = *preg_s;
                *preg_c8 &= ~((uint8_t) (*pvalue & 0xFF));
                vgicd_changed_istatus(vcpuid, *preg_c, (offset >> 2) - __GICD_ICENABLER, old_status);
            } else {
                *pvalue = (uint32_t) * preg_c8;
            }

            result = HVMM_STATUS_SUCCESS;
        }
    }
    return result;
}

static hvmm_status_t handler_ISCPENDR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t *preg_s;
    uint32_t *preg_c;

    if (((offset >> 2) - __GICD_ISPENDR) == 0 ||
            ((offset >> 2) - __GICD_ICPENDR) == 0) {
        preg_s = &(regs_banked->ISCPENDR);
        preg_c = &(regs_banked->ISCPENDR);
    } else {
        preg_s = &(regs->ISCPENDR[(offset >> 2) - __GICD_ISPENDR]);
        preg_c = &(regs->ISCPENDR[(offset >> 2) - __GICD_ICPENDR]);
    }
    offset >>= 2;
    if (access_size == VDEV_ACCESS_WORD) {
        if ((offset >> 2) < (__GICD_ISPENDR + VGICD_NUM_IENABLER)) {
            /* ISPEND */
            if (write) {
                *preg_s |= *pvalue;
            } else {
                *pvalue = *preg_s;
            }
            result = HVMM_STATUS_SUCCESS;
        } else if ((offset >> 2) >= __GICD_ICPENDR
                   && (offset >> 2)
                   < (__GICD_ICPENDR + VGICD_NUM_IENABLER)) {
            /* ICPEND */
            if (write) {
                *preg_c &= ~(*pvalue);
            } else {
                *pvalue = *preg_c;
            }
            result = HVMM_STATUS_SUCCESS;
        }
    }
    printf("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_ISCACTIVER(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printf("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    uint32_t *preg;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;

    /* FIXME: Support 8/16/32bit access */
    offset >>= 2;
    if ((offset - __GICD_IPRIORITYR) < VGICD_BANKED_NUM_IPRIORITYR) {
        preg = &(regs_banked->ICFGR);
    } else {
        preg = &(regs->IPRIORITYR[offset - __GICD_IPRIORITYR]);
    }

    if (write) {
        *preg = *pvalue;
    } else {
        *pvalue = *preg;
    }

    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t *preg;

    if (((offset >> 2) - __GICD_ITARGETSR) < VGICD_BANKED_NUM_ITARGETSR) {
        preg = &(regs_banked->ITARGETSR[(offset >> 2) - __GICD_ITARGETSR]);
    } else {
        preg = &(regs->ITARGETSR[(offset >> 2) - __GICD_ITARGETSR]);
    }

    if (access_size == VDEV_ACCESS_WORD) {
        offset >>= 2;
        if (write) {
            if (offset > (__GICD_ITARGETSR + 7)) {
                /* RO: ITARGETSR0 ~ 7 */
                *preg = *pvalue;
            }
        } else {
            *pvalue = *preg;
        }
    } else if (access_size == VDEV_ACCESS_HWORD) {
        uint16_t *preg16 = (uint16_t *) preg;
        preg16 += (offset & 0x3) >> 1;
        if (write) {
            if ((offset >> 2) > (__GICD_ITARGETSR + 7)) {
                *preg16 = (uint16_t) (*pvalue & 0xFFFF);
            }
        } else {
            *pvalue = (uint32_t) * preg16;
        }
    } else if (access_size == VDEV_ACCESS_BYTE) {
        uint8_t *preg8 = (uint8_t *) preg;
        preg8 += (offset & 0x3);
        if (write) {
            if ((offset >> 2) > (__GICD_ITARGETSR + 7)) {
                *preg8 = (uint8_t) (*pvalue & 0xFF);
            }
        } else {
            *pvalue = (uint32_t) * preg8;
        }
    }
    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t *preg;

    /* FIXME: Support 8/16/32bit access */
    offset >>= 2;
    if ((offset - __GICD_ICFGR) == 1) {
        preg = &(regs_banked->ICFGR);
    } else {
        preg = &(regs->ICFGR[offset - __GICD_ICFGR]);
    }

    if (write) {
        *preg = *pvalue;
    } else {
        *pvalue = *preg;
    }
    result = HVMM_STATUS_SUCCESS;
    return result;
}

static hvmm_status_t handler_PPISPISR_CA15(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printf("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_NSACR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    printf("vgicd:%s: not implemented\n", __func__);
    return result;
}

static hvmm_status_t handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs_banked *regs_banked;
    uint32_t target = 0;
    uint32_t sgi_id = *pvalue & GICD_SGIR_SGI_INT_ID_MASK;
    uint32_t i;
    uint32_t *preg_s;
    uint32_t *preg_c;

    if (((offset >> 2) == __GICD_CPENDSGIR) ||
            ((offset >> 2) == __GICD_SPENDSGIR)) {
        regs_banked = &vm->vgic.gicd_regs_banked;
        preg_s = &(regs_banked->CPENDSGIR[(offset >> 2) - __GICD_SPENDSGIR]);
        preg_c = &(regs_banked->CPENDSGIR[(offset >> 2) - __GICD_CPENDSGIR]);
    }
    offset >>= 2;

    if (offset == __GICD_SGIR) {
        switch (*pvalue & GICD_SGIR_TARGET_LIST_FILTER_MASK) {
        case GICD_SGIR_TARGET_LIST:
            target = ((*pvalue & GICD_SGIR_CPU_TARGET_LIST_MASK) >> GICD_SGIR_CPU_TARGET_LIST_OFFSET);
            break;
        case GICD_SGIR_TARGET_OTHER:
            target = ~(0x1 << vcpu->vmid);
            break;
        case GICD_SGIR_TARGET_SELF:
            target = (0x1 << vcpu->vmid);
            break;
        default:
            //printf();
            return result;
        }
        dsb();

        for (i = 0; i < NUM_GUESTS_STATIC; i++) {
            uint8_t _target = target & 0x1;
            if (_target) {
                vm = vm_find(_target);
                regs_banked = &vm->vgic.gicd_regs_banked;
                (regs_banked -> CPENDSGIR[(sgi_id >> 2)]) = 0x1 << ((sgi_id & 0x3) * 8);
                result = virq_inject(i, sgi_id, sgi_id, 0);
            }
            target = target >> 1;
        }
    } else if (offset == __GICD_CPENDSGIR) {
        if (write) {
            if (*pvalue) {
                *preg_c |= ~(*pvalue);
            }
        } else { // read
            *pvalue = *preg_c;
        }
        result = HVMM_STATUS_SUCCESS;
    } else if (offset == __GICD_SPENDSGIR) {
        if (write) {
            if (*pvalue) {
                *preg_s |= *pvalue;
            }
        } else { // read
            *pvalue = *preg_s;
        }
        result = HVMM_STATUS_SUCCESS;
    } else { //ICPIDR2 is not implemented
        printf("vgicd:%s: not implemented\n", __func__);
    }
    return result;
}

static int32_t vdev_gicd_write_handler(struct arch_vdev_trigger_info *info, struct core_regs *regs)
{
    uint32_t *pvalue = info->value;
    enum vdev_access_size access_size = info->sas;
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    switch (offset)
    {
        case GICD_CTLR_OFFSET:
            return handler_000(WRITE, offset, pvalue, access_size);

        case GICD_TYPER_OFFSET:
            return handler_000(WRITE, offset, pvalue, access_size);

        case GICD_IIDR_OFFSET:
            return handler_000(WRITE, offset, pvalue, access_size);

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
            return handler_000(WRITE, offset, pvalue, access_size);

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
            return handler_ISCENABLER(WRITE, offset, pvalue, access_size);

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
            return handler_ISCENABLER(WRITE, offset, pvalue, access_size);

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
            return handler_ISCPENDR(WRITE, offset, pvalue, access_size);

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
            return handler_ISCPENDR(WRITE, offset, pvalue, access_size);

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
            return handler_ISCACTIVER(WRITE, offset, pvalue, access_size);

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
            return handler_ISCACTIVER(WRITE, offset, pvalue, access_size);

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
            return handler_IPRIORITYR(WRITE, offset, pvalue, access_size);

        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
            return handler_ITARGETSR(WRITE, offset, pvalue, access_size);

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
            return handler_ICFGR(WRITE, offset, pvalue, access_size);

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            return handler_NSACR(WRITE, offset, pvalue, access_size);

        case GICD_SGIR:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case 0xD00 ... 0xDFC:
            return handler_PPISPISR_CA15(WRITE, offset, pvalue, access_size);

        default:
            printf("there's no corresponding address\n");
            return HVMM_STATUS_BAD_ACCESS;
    }

    return HVMM_STATUS_SUCCESS;
}

static int32_t vdev_gicd_read_handler(struct arch_vdev_trigger_info *info, struct core_regs *regs)
{
    uint32_t *pvalue = info->value;
    enum vdev_access_size access_size = info->sas;
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    switch (offset)
    {
        case GICD_CTLR_OFFSET:
            return handler_000(READ, offset, pvalue, access_size);

        case GICD_TYPER_OFFSET:
            return handler_000(READ, offset, pvalue, access_size);

        case GICD_IIDR_OFFSET:
            return handler_000(READ, offset, pvalue, access_size);

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
            return handler_000(READ, offset, pvalue, access_size);

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
            return handler_ISCENABLER(READ, offset, pvalue, access_size);

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
            return handler_ISCENABLER(READ, offset, pvalue, access_size);

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
            return handler_ISCPENDR(READ, offset, pvalue, access_size);

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
            return handler_ISCPENDR(READ, offset, pvalue, access_size);

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
            return handler_ISCACTIVER(READ, offset, pvalue, access_size);

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
            return handler_ISCACTIVER(READ, offset, pvalue, access_size);

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
            return handler_IPRIORITYR(READ, offset, pvalue, access_size);

        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
            return handler_ITARGETSR(READ, offset, pvalue, access_size);

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
            return handler_ICFGR(READ, offset, pvalue, access_size);

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            return handler_NSACR(READ, offset, pvalue, access_size);

        case GICD_SGIR:
            return handler_F00(READ, offset, pvalue, access_size);

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
            return handler_F00(READ, offset, pvalue, access_size);

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
            return handler_F00(READ, offset, pvalue, access_size);

        case 0xD00 ... 0xDFC:
            return handler_PPISPISR_CA15(READ, offset, pvalue, access_size);

        default:
            printf("there's no corresponding address\n");
            return HVMM_STATUS_BAD_ACCESS;
    }

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t vdev_gicd_post(struct arch_vdev_trigger_info *info, struct core_regs *regs)
{
    uint8_t isize = 4;

    if (regs->cpsr & 0x20) { /* Thumb */
        isize = 2;
    }

    regs->pc += isize;

    return 0;
}

static int32_t vdev_gicd_check(struct arch_vdev_trigger_info *info, struct core_regs *regs)
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
    printf("Dummy vdev_gicd_init \n");
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

