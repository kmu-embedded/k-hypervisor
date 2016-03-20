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

static hvmm_status_t handler_IPRIORITYR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ITARGETSR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_ICFGR(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);
static hvmm_status_t handler_F00(uint32_t write, uint32_t offset, uint32_t *pvalue, enum vdev_access_size access_size);

static struct vdev_memory_map _vdev_gicd_info = {
    .base = CFG_GIC_BASE_PA | GICD_OFFSET,
    .size = 4096,
};

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
        preg_s = &(regs_banked->SPENDSGIR[(offset >> 2) - __GICD_SPENDSGIR]);
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

static int32_t vdev_gicd_write_handler(struct arch_vdev_trigger_info *info, struct core_regs *core_regs)
{
    uint32_t *pvalue = info->value;
    enum vdev_access_size access_size = info->sas;
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);

    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;
    uint32_t old_status;

    switch (offset)
    {
        case GICD_CTLR_OFFSET:
            regs->CTLR = *pvalue;
            return HVMM_STATUS_SUCCESS;

        case GICD_TYPER_OFFSET:
            return HVMM_STATUS_SUCCESS;

        case GICD_IIDR_OFFSET:
            return HVMM_STATUS_SUCCESS;

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
        {
            int igroup = (offset - GICD_IGROUPR(0)) >> 2;

            if ((igroup == 0) && (igroup < VGICD_NUM_IGROUPR))
                *pvalue = regs_banked->IGROUPR;
            else if ((igroup > 0) && (igroup < VGICD_NUM_IGROUPR))
                *pvalue = regs->IGROUPR[igroup];

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ISENABLER(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ISENABLER);
            } else {
                preg_s = &(regs->ISENABLER[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                old_status = *preg_s;
                *preg_s |= *pvalue;
                vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ISENABLER(0)) >> 2, old_status);
            } else if (access_size == VDEV_ACCESS_HWORD) {
                uint16_t *preg_s16 = (uint16_t *) preg_s;
                preg_s16 += (offset & 0x3) >> 1;
                old_status = *preg_s;
                *preg_s16 |= (uint16_t) (*pvalue & 0xFFFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ISENABLER(0)) >> 2, old_status);
            } else if (access_size == VDEV_ACCESS_BYTE) {
                uint8_t *preg_s8 = (uint8_t *) preg_s;
                preg_s8 += (offset & 0x3);
                old_status = *preg_s;
                *preg_s8 |= (uint8_t) (*pvalue & 0xFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ISENABLER(0)) >> 2, old_status);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ICENABLER(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ICENABLER);
            } else {
                preg_s = &(regs->ICENABLER[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                    old_status = *preg_s;
                    *preg_s |= *pvalue;
                    vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ICENABLER(0)) >> 2, old_status);
            } else if (access_size == VDEV_ACCESS_HWORD) {
                uint16_t *preg_s16 = (uint16_t *) preg_s;
                preg_s16 += (offset & 0x3) >> 1;
                old_status = *preg_s;
                *preg_s16 |= (uint16_t) (*pvalue & 0xFFFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ICENABLER(0)) >> 2, old_status);
            } else if (access_size == VDEV_ACCESS_BYTE) {
                uint8_t *preg_s8 = (uint8_t *) preg_s;
                preg_s8 += (offset & 0x3);
                old_status = *preg_s;
                *preg_s8 |= (uint8_t) (*pvalue & 0xFF);
                vgicd_changed_istatus(vcpuid, *preg_s, (offset - GICD_ICENABLER(0)) >> 2, old_status);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ISPENDR(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ISPENDR);
            } else {
                preg_s = &(regs->ISPENDR[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *preg_s |= *pvalue;
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ICPENDR(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ICPENDR);
            } else {
                preg_s = &(regs->ICPENDR[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *preg_s &= ~(*pvalue);
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
            printf("vgicd: GICD_ISATIVER wite not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
            printf("vgicd: GICD_ICATIVER wite not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
            return handler_IPRIORITYR(WRITE, offset, pvalue, access_size);

        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
            return handler_ITARGETSR(WRITE, offset, pvalue, access_size);

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
            return handler_ICFGR(WRITE, offset, pvalue, access_size);

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            printf("vgicd: GICD_NSACR write not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_SGIR:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
            return handler_F00(WRITE, offset, pvalue, access_size);

        case 0xD00 ... 0xDFC:
            printf("vgicd: GICD_PPISPISR_CA15 write not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        default:
            printf("there's no corresponding address\n");
            return HVMM_STATUS_BAD_ACCESS;
    }

    return HVMM_STATUS_SUCCESS;
}

static int32_t vdev_gicd_read_handler(struct arch_vdev_trigger_info *info, struct core_regs *core_regs)
{
    uint32_t *pvalue = info->value;
    enum vdev_access_size access_size = info->sas;
    uint32_t offset = info->fipa - _vdev_gicd_info.base;

    vcpuid_t vcpuid = get_current_vcpuid();
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct vmcb *vm = vm_find(vcpu->vmid);
    struct gicd_regs *regs = &vm->vgic.gicd_regs;
    struct gicd_regs_banked *regs_banked = &vm->vgic.gicd_regs_banked;

    switch (offset)
    {
        case GICD_CTLR_OFFSET:
            *pvalue = regs->CTLR;
            return HVMM_STATUS_SUCCESS;

        case GICD_TYPER_OFFSET:
            *pvalue = regs->TYPER;
            return HVMM_STATUS_SUCCESS;

        case GICD_IIDR_OFFSET:
            *pvalue = regs->IIDR;
            return HVMM_STATUS_SUCCESS;

        case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST:
        {
            int igroup = (offset - GICD_IGROUPR(0)) >> 2;

            if ((igroup == 0) && (igroup < VGICD_NUM_IGROUPR))
                *pvalue = regs_banked->IGROUPR;
            else if ((igroup > 0) && (igroup < VGICD_NUM_IGROUPR))
                *pvalue = regs->IGROUPR[igroup];

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ISENABLER(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ISENABLER);
            } else {
                preg_s = &(regs->ISENABLER[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *pvalue = *preg_s;
            } else if (access_size == VDEV_ACCESS_HWORD) {
                uint16_t *preg_s16 = (uint16_t *) preg_s;
                preg_s16 += (offset & 0x3) >> 1;
                *pvalue = (uint32_t) * preg_s16;
            } else if (access_size == VDEV_ACCESS_BYTE) {
                uint8_t *preg_s8 = (uint8_t *) preg_s;
                preg_s8 += (offset & 0x3);
                *pvalue = (uint32_t) * preg_s8;
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ICENABLER(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ICENABLER);
            } else {
                preg_s = &(regs->ICENABLER[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *pvalue = *preg_s;
            } else if (access_size == VDEV_ACCESS_HWORD) {
                uint16_t *preg_s16 = (uint16_t *) preg_s;
                preg_s16 += (offset & 0x3) >> 1;
                *pvalue = (uint32_t) * preg_s16;
            } else if (access_size == VDEV_ACCESS_BYTE) {
                uint8_t *preg_s8 = (uint8_t *) preg_s;
                preg_s8 += (offset & 0x3);
                *pvalue = (uint32_t) * preg_s8;
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ISPENDR(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ISPENDR);
            } else {
                preg_s = &(regs->ISPENDR[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *pvalue = *preg_s;
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST:
        {
            uint32_t *preg_s;

            if ((offset  - GICD_ICPENDR(0)) >> 2 == 0 ) {
                preg_s = &(regs_banked->ICPENDR);
            } else {
                preg_s = &(regs->ICPENDR[(offset >> 2)]);
            }

            if (access_size == VDEV_ACCESS_WORD) {
                *pvalue = *preg_s;
            }

            return HVMM_STATUS_SUCCESS;
        }

        case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST:
            printf("vgicd: GICD_ISACTIVER read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST:
            printf("vgicd: GICD_ICACTIVER read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST:
            return handler_IPRIORITYR(READ, offset, pvalue, access_size);

        case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST:
            return handler_ITARGETSR(READ, offset, pvalue, access_size);

        case GICD_ICFGR(0) ... GICD_ICFGR_LAST:
            return handler_ICFGR(READ, offset, pvalue, access_size);

        case GICD_NSACR(0) ... GICD_NSACR_LAST:
            printf("vgicd: GICD_NSACR read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

        case GICD_SGIR:
            return handler_F00(READ, offset, pvalue, access_size);

        case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST:
            return handler_F00(READ, offset, pvalue, access_size);

        case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST:
            return handler_F00(READ, offset, pvalue, access_size);

        case 0xD00 ... 0xDFC:
            printf("vgicd: GICD_PPISPISR_CA15 read not implemented\n", __func__);
            return HVMM_STATUS_BAD_ACCESS;

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

