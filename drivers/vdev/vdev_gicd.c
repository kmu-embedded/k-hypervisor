#include <arch/gic_regs.h>
#include <vdev.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdio.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>
#include <drivers/gic-v2.h>
#include <drivers/vdev/vdev_gicd.h>

int32_t vgicd_read_handler(void *pdata, uint32_t offset);
int32_t vgicd_write_handler(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vgicd_create_instance(void **pdata);

struct vdev_module vdev_gicd = {
    .name = "vgicd_v2",
    .base = CFG_GIC_BASE_PA | GICD_OFFSET,
    .size = 0x1000,
    .read = vgicd_read_handler,
    .write = vgicd_write_handler,
    .create = vgicd_create_instance,
};

union sgir {
    uint32_t raw: 32;
    struct {
        uint32_t id: 4;
        uint32_t sbz: 11;
        uint32_t nsatt: 1;
        uint32_t CPUTargetList: 8;
        uint32_t TargetListFilter: 2;
        uint32_t reserved: 6;
    } entry;
};
typedef union sgir sgir_t;

#include <stdlib.h>
#include <string.h>
int32_t vgicd_create_instance(void **pdata)
{
    struct vgicd *vgicd = malloc(sizeof(struct vgicd));
    int i, j;

    memset(vgicd, 0, sizeof(struct vgicd));

    vgicd->typer = GICD_READ(GICD_TYPER);
    vgicd->iidr  = GICD_READ(GICD_IIDR);

    // Initialize GICD_ITARGETSR
    for (i = 0; i < NR_VCPUS; i++) {
        for (j = 0; j < NR_BANKED_ITARGETSR; j++) {
            vgicd->itargetsr0[i][j] = GICD_READ(GICD_ITARGETSR(j));
        }
    }

    for (i = 0; i < NR_ITARGETSR; i++) {
        vgicd->itargetsr[i] = 1 << 0 | 1 << 8 | 1 << 16 | 1 << 24;
    }

    *pdata = vgicd;

    return 0;
}

#define firstbit32(word) (31 - asm_clz(word))

static uint32_t ITLinesNumber = 0;

static void set_enable(uint32_t current_status, uint8_t n, uint32_t old_status)
{
    uint32_t delta = old_status ^ current_status;

    struct vcpu *vcpu = get_current_vcpu();

    while (delta) {
        uint32_t offset = firstbit32(delta);
        uint32_t virq = offset + (32 * n);
        uint32_t pirq = virq_to_pirq(vcpu, virq);

        pirq = (pirq == PIRQ_INVALID ? virq : pirq);
        virq_enable(vcpu, pirq, virq);
        pirq_enable(vcpu, pirq, virq);
        printf("enable irq[%d]\n", pirq);
        gic_enable_irq(pirq);

        delta &= ~(1 << offset);
    }
}

static void set_clear(uint32_t current_status, uint8_t n, uint32_t old_status)
{
    uint32_t delta = old_status ^ current_status;
    struct vcpu *vcpu = get_current_vcpu();

    while (delta) {

        uint32_t offset = firstbit32(delta);
        uint32_t virq = offset + (32 * n);
        uint32_t pirq = virq_to_pirq(vcpu, virq);

        pirq = (pirq == PIRQ_INVALID ? virq : pirq);
        virq_disable(vcpu, virq);
        pirq_disable(vcpu, pirq);
        // TODO(casionwoo) : When VM try to interrupt clear, must be checked every VM clear the interrupt. Then clear the irq
        // gic_disable_irq(pirq);

        delta &= ~(1 << offset);
    }
}

static void handler_SGIR(void *pdata, uint32_t offset, uint32_t value)
{
    struct vcpu *vcpu = get_current_vcpu();
    struct vmcb *vm = get_current_vm();
    struct vcpu *target_vcpu;
    uint32_t pcpu = smp_processor_id();

    sgir_t sgi;
    sgi.raw = value;

    switch (sgi.entry.TargetListFilter) {

    case 0:
        while (sgi.entry.CPUTargetList) {
            uint8_t target_vcpuid = firstbit32(sgi.entry.CPUTargetList);

            if (target_vcpuid < vm->num_vcpus) {
                target_vcpu = vm->vcpu[target_vcpuid];
                sgi.entry.CPUTargetList &= ~(1 << target_vcpuid);
                virq_inject(target_vcpu, sgi.entry.id, sgi.entry.id, SW_IRQ);
            }
        }
        break;

    case 1: {
        int cpuid;

        for (cpuid = 0; cpuid < vm->num_vcpus; cpuid++) {
            if (cpuid == pcpu) {
                continue;
            }

            target_vcpu = vm->vcpu[cpuid];
            virq_inject(target_vcpu, sgi.entry.id, sgi.entry.id, SW_IRQ);
        }
    }
    break;

    case 2:
        virq_inject(vcpu, sgi.entry.id, sgi.entry.id, SW_IRQ);
        break;

    default:
        printf("Reserved case of SGIR TargetListFilter :%d\n", sgi.entry.TargetListFilter);
    }
}

int32_t vgicd_write_handler(void *pdata, uint32_t offset, uint32_t *addr)
{
    struct vgicd *gicd = pdata;
    uint8_t vcpuid = get_current_vcpuidx();

    uint32_t old_status;

    switch (offset) {
    case GICD_CTLR:
        gicd->ctlr = readl(addr);
        break;

    case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST: {
        uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

        if (n == 0) {
            gicd->igroupr0[vcpuid] = readl(addr);
        } else if ((n > 0) && (n < (ITLinesNumber + 1))) {
            gicd->igroupr[n] = readl(addr);
        }
    }
    break;

    case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST: {
        uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

        if (n == 0) {
            old_status = gicd->isenabler0[vcpuid];
            gicd->isenabler0[vcpuid] |= readl(addr);
            set_enable(gicd->isenabler0[vcpuid], n, old_status);
        } else {
            old_status = gicd->isenabler[n];
            gicd->isenabler[n] |= readl(addr);
            set_enable(gicd->isenabler[n], n, old_status);
        }
    }
    break;

    case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST: {
        uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

        if (n == 0) {
            old_status = gicd->icenabler0[vcpuid];
            gicd->icenabler0[vcpuid] |= readl(addr);
            set_clear(gicd->icenabler0[vcpuid], n, old_status);
        } else {
            old_status = gicd->icenabler[n];
            gicd->icenabler[n] |= readl(addr);
            set_clear(gicd->icenabler[n], n, old_status);
        }
    }
    break;

    case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST: {
        uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

        if (n == 0) {
            gicd->ispendr0[vcpuid] |= readl(addr);
        } else {
            gicd->ispendr[n] |= readl(addr);
        }
    }
    break;

    case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST: {
        uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

        if (n == 0) {
            gicd->icpendr0[vcpuid] = ~(readl(addr));

        } else {
            gicd->icpendr[n] = ~(readl(addr));
        }
    }
    break;

    case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST: {
        uint32_t n = (offset - GICD_ISACTIVER(0)) >> 2;

        // Overwrite has no effect.
        GICD_WRITE(GICD_ISACTIVER(n), readl(addr));

    }
    break;

    case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST: {
        uint32_t n = (offset - GICD_ICACTIVER(0)) >> 2;

        GICD_WRITE(GICD_ICACTIVER(n), readl(addr));

    }
    break;

    case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST: {
        uint32_t n = ((offset - GICD_IPRIORITYR(0)) >> 2);

        if (n < NR_BANKED_IPRIORITYR) {
            gicd->ipriorityr0[vcpuid][n] = readl(addr);
        } else {
            gicd->ipriorityr[n] = readl(addr);
        }
    }
    break;

    case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST: {
        uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;

        if (n < NR_BANKED_ITARGETSR) {
            gicd->itargetsr0[vcpuid][n] |= readl(addr);
        } else {
            gicd->itargetsr[n] |= readl(addr);
        }
    }
    break;

    case GICD_ICFGR(0) ... GICD_ICFGR_LAST: {
        uint32_t n = (offset - GICD_ICFGR(0));

        if (n == 1) {
            gicd->icfgr0[vcpuid] = readl(addr);
        } else {
            gicd->icfgr[n] = readl(addr);
        }
    }
    break;

    case GICD_NSACR(0) ... GICD_NSACR_LAST:
        printf("vgicd: GICD_NSACR write not implemented\n", __func__);
        break;

    case GICD_SGIR:
        handler_SGIR(pdata, offset, readl(addr));
        break;

    case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST: {

        uint32_t n = offset - GICD_CPENDSGIR(0);
        gicd->cpendsgir0[vcpuid][n] = ~(readl(addr));

    }
    break;

    case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST: {

        uint32_t n = offset - GICD_SPENDSGIR(0);
        gicd->spendsgir0[vcpuid][n] = ~(readl(addr));
    }
    break;

    case 0xD00 ... 0xDFC:
        printf("vgicd: GICD_PPISPISR_CA15 write not implemented\n", __func__);
        break;
    default:
        printf("there's no corresponding address or read only register\n");
        break;
    }

    return 0;
}

int32_t vgicd_read_handler(void *pdata, uint32_t offset)
{
    struct vgicd *gicd = pdata;
    uint8_t vcpuid = get_current_vcpuidx();

    switch (offset) {
    case GICD_CTLR:
        return gicd->ctlr;
        break;

    case GICD_TYPER:
        return gicd->typer;
        break;

    case GICD_IIDR:
        return gicd->iidr;
        break;

    case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST: {
        uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

        if (n == 0) {
            return gicd->igroupr0[vcpuid];
        } else if ((n > 0) && (n < (ITLinesNumber + 1))) {
            return gicd->igroupr[n];
        }
    }
    break;

    case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST: {
        uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

        if (n == 0) {
            return gicd->isenabler0[vcpuid];
        } else {
            return gicd->isenabler[n];
        }
    }
    break;

    case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST: {
        uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

        if (n == 0) {
            return gicd->icenabler0[vcpuid];
        } else {
            return gicd->icenabler[n];
        }
    }
    break;

    case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST: {
        uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

        if (n == 0) {
            return gicd->ispendr0[vcpuid];
        } else {
            return gicd->ispendr[n];
        }
    }
    break;

    case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST: {
        uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

        if (n == 0) {
            return gicd->icpendr0[vcpuid];
        } else {
            return gicd->icpendr[n];
        }
    }
    break;

    case GICD_ISACTIVER(0) ... GICD_ISACTIVER_LAST: {
        uint32_t n = (offset - GICD_ISACTIVER(0)) >> 2;
        return GICD_READ(GICD_ISACTIVER(n));
    }
    break;

    case GICD_ICACTIVER(0) ... GICD_ICACTIVER_LAST: {
        uint32_t n = (offset - GICD_ICACTIVER(0)) >> 2;
        return GICD_READ(GICD_ICACTIVER(n));
    }
    break;

    case GICD_IPRIORITYR(0) ... GICD_IPRIORITYR_LAST: {
        uint32_t n = ((offset - GICD_IPRIORITYR(0)) >> 2);
        if (n < NR_BANKED_IPRIORITYR) {
            return gicd->ipriorityr0[vcpuid][n];
        } else {
            return gicd->ipriorityr[n];

        }
    }
    break;

    case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST: {
        uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;
        if (n < NR_BANKED_ITARGETSR) {
            return gicd->itargetsr0[vcpuid][n];
        } else {
            return gicd->itargetsr[n];
        }
    }
    break;

    case GICD_ICFGR(0) ... GICD_ICFGR_LAST: {
        uint32_t n = (offset - GICD_ICFGR(0));
        if (n == 1) {
            return gicd->icfgr0[vcpuid];
        } else {
            return gicd->icfgr[n];
        }
    }
    break;

    case GICD_NSACR(0) ... GICD_NSACR_LAST:
        return HVMM_STATUS_BAD_ACCESS;

        break;

    case GICD_SGIR:
        return 0;

        break;

    case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST: {
        return gicd->cpendsgir0[vcpuid][offset - GICD_CPENDSGIR(0)];
    }
    break;

    case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST: {
        return gicd->spendsgir0[vcpuid][offset - GICD_SPENDSGIR(0)];
    }
    break;

    case 0xD00 ... 0xDFC:
        printf("vgicd: GICD_PPISPISR_CA15 read not implemented\n", __func__);
        return 0;
        break;

    default:
        printf("there's no corresponding address\n");
        break;
    }

    return 0;
}

hvmm_status_t vdev_gicd_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    vdev_register(&vdev_gicd);
    ITLinesNumber = GICv2.ITLinesNumber;

    printf("vdev registered:'%s'\n", vdev_gicd.name);

    return result;
}

vdev_module_init(vdev_gicd_init);

