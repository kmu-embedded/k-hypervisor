#include <arch/gic_regs.h>
#include <vdev.h>
#include <arch/armv7.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdio.h>
#include <core/vm.h>
#include <core/vm/vcpu.h>
#include "../../drivers/gic-v2.h"
#include <core/vm/virq.h>

#define READ    0
#define WRITE   1

struct vdev_module vgicd;

#define firstbit32(word) (31 - asm_clz(word))

static uint32_t ITLinesNumber = 0;

static void set_enable(struct vcpu* vcpu, uint32_t current_status, uint8_t n, uint32_t old_status) {
	uint32_t delta = old_status ^ current_status;

	while (delta) {
		uint32_t offset = firstbit32(delta);
		uint32_t virq = offset + (32 * n);
		uint32_t pirq = virq_to_pirq(vcpu, virq);

		pirq = (pirq == PIRQ_INVALID ? virq : pirq);

		virq_enable(vcpu, pirq, virq);
		pirq_enable(vcpu, pirq, virq);
		gic_enable_irq(pirq);

		delta &= ~(1 << offset);
	}
}

static void set_clear(struct vcpu *vcpu, uint32_t current_status, uint8_t n, uint32_t old_status) {
	uint32_t delta = old_status ^ current_status;

	while (delta) {

		uint32_t offset = firstbit32(delta);
		uint32_t virq = offset + (32 * n);
		uint32_t pirq = virq_to_pirq(vcpu, virq);

		virq_disable(vcpu, virq);
		pirq_disable(vcpu, pirq);
		// TODO(casionwoo) : When VM try to interrupt clear, must be checked every VM clear the interrupt. Then clear the irq
		// gic_disable_irq(pirq);

		delta &= ~(1 << offset);
	}
}

static hvmm_status_t handler_SGIR(struct vcpu *vcpu, uint32_t offset, uint32_t value) {
	hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
	struct banked_virq *gicd_banked;

	uint32_t target_cpu_interfaces = 0;
	uint32_t sgi_id = value & GICD_SGIR_SGI_INT_ID_MASK;
	uint8_t target_vcpuid;

	switch (value & GICD_SGIR_TARGET_LIST_FILTER_MASK) {
	case GICD_SGIR_TARGET_LIST:
		target_cpu_interfaces = ((value & GICD_SGIR_CPU_TARGET_LIST_MASK)
				>> GICD_SGIR_CPU_TARGET_LIST_OFFSET);
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

			gicd_banked = &vcpu->banked_virq;
			gicd_banked->SPENDSGIR[n] = 0x1
					<< ((reg_offset * 8) + target_cpu_interface);
			result = virq_inject(target_vcpuid, sgi_id, sgi_id, SW_IRQ);
		}

		target_vcpuid++;
		target_cpu_interfaces = target_cpu_interfaces >> 1;
	}

	return result;
}

int32_t vgicd_write_handler(uint32_t offset, uint32_t *addr) {
	offset -= vgicd.base;
	struct vcpu *vcpu = get_current_vcpu();
	struct vmcb *vm = get_current_vm();
	struct virq *gicd = &vm->virq;
	struct banked_virq *gicd_banked = &vcpu->banked_virq;

	uint32_t old_status;
	//printf("VDEV WRITE: offset %x\n", offset);

	switch (offset) {
	case GICD_CTLR:
		gicd->CTLR = readl(addr);
		break;

	case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST: {
		uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

		if (n == 0) {
			gicd_banked->IGROUPR = readl(addr);
		} else if ((n > 0) && (n < (ITLinesNumber + 1))) {
			gicd->IGROUPR[n] = readl(addr);
		}
	}
		break;

	case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST: {
		uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

		if (n == 0) {
			old_status = gicd_banked->ISENABLER;
			gicd_banked->ISENABLER |= readl(addr);
			set_enable(vcpu, gicd_banked->ISENABLER, n, old_status);
		} else {
			old_status = gicd->ISENABLER[n];
			gicd->ISENABLER[n] |= readl(addr);
			set_enable(vcpu, gicd->ISENABLER[n], n, old_status);
		}
	}
		break;

	case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST: {
		uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

		if (n == 0) {
			old_status = gicd_banked->ICENABLER;
			gicd_banked->ICENABLER |= readl(addr);
			set_clear(vcpu, gicd_banked->ICENABLER, n, old_status);
		} else {
			old_status = gicd->ICENABLER[n];
			gicd->ICENABLER[n] |= readl(addr);
			set_clear(vcpu, gicd->ICENABLER[n], n, old_status);
		}
	}
		break;

	case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST: {
		uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

		if (n == 0) {
			gicd_banked->ISPENDR |= readl(addr);
		} else {
			gicd->ISPENDR[n] |= readl(addr);
		}
	}
		break;

	case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST: {
		uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

		if (n == 0) {
			gicd_banked->ICPENDR = ~(readl(addr));

		} else {
			gicd->ICPENDR[n] = ~(readl(addr));
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
			gicd_banked->IPRIORITYR[n] = readl(addr);
		} else {
			gicd->IPRIORITYR[n] = readl(addr);
		}
	}
		break;

	case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST: {
		uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;

		if (n < NR_BANKED_ITARGETSR) {
			gicd_banked->ITARGETSR[n] |= readl(addr);
		} else {
			gicd->ITARGETSR[n] |= readl(addr);
		}
	}
		break;

	case GICD_ICFGR(0) ... GICD_ICFGR_LAST: {
		uint32_t n = (offset - GICD_ICFGR(0));

		if (n == 1) {
			gicd_banked->ICFGR = readl(addr);
		} else {
			gicd->ICFGR[n] = readl(addr);
		}
	}
		break;

	case GICD_NSACR(0) ... GICD_NSACR_LAST:
		printf("vgicd: GICD_NSACR write not implemented\n", __func__);
		break;

	case GICD_SGIR:
		handler_SGIR(vcpu, offset, readl(addr));
		break;

	case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST: {

		uint32_t n = offset - GICD_CPENDSGIR(0);
		gicd_banked->CPENDSGIR[n] = ~(readl(addr));

	}
		break;

	case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST: {

		uint32_t n = offset - GICD_SPENDSGIR(0);
		gicd_banked->SPENDSGIR[n] = ~(readl(addr));
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

int32_t vgicd_read_handler(uint32_t offset, uint32_t *addr) {
	offset -= vgicd.base;
	struct vcpu *vcpu = get_current_vcpu();
	struct vmcb *vm = get_current_vm();
	struct virq *gicd = &vm->virq;
	struct banked_virq *gicd_banked = &vcpu->banked_virq;

	switch (offset) {
	case GICD_CTLR:
		return gicd->CTLR;
		break;

	case GICD_TYPER:
		return gicd->TYPER;
		break;

	case GICD_IIDR:
		return gicd->IIDR;
		break;

	case GICD_IGROUPR(0) ... GICD_IGROUPR_LAST: {
		uint32_t n = (offset - GICD_IGROUPR(0)) >> 2;

		if (n == 0) {
			return gicd_banked->IGROUPR;
		} else if ((n > 0) && (n < (ITLinesNumber + 1))) {
			return gicd->IGROUPR[n];
		}
	}
	break;

	case GICD_ISENABLER(0) ... GICD_ISENABLER_LAST: {
		uint32_t n = (offset - GICD_ISENABLER(0)) >> 2;

		if (n == 0) {
			return gicd_banked->ISENABLER;
		} else {
			return gicd->ISENABLER[n];
		}
	}
	break;

	case GICD_ICENABLER(0) ... GICD_ICENABLER_LAST: {
		uint32_t n = (offset - GICD_ICENABLER(0)) >> 2;

		if (n == 0) {
			return gicd_banked->ICENABLER;
		} else {
			return gicd->ICENABLER[n];
		}
	}
	break;

	case GICD_ISPENDR(0) ... GICD_ISPENDR_LAST: {
		uint32_t n = (offset - GICD_ISPENDR(0)) >> 2;

		if (n == 0) {
			return gicd_banked->ISPENDR;
		} else {
			return gicd->ISPENDR[n];
		}
	}
	break;

	case GICD_ICPENDR(0) ... GICD_ICPENDR_LAST: {
		uint32_t n = (offset - GICD_ICPENDR(0)) >> 2;

		if (n == 0) {
			return gicd_banked->ICPENDR;
		} else {
			return gicd->ICPENDR[n];
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
			return gicd_banked->IPRIORITYR[n];
		} else {
			return gicd->IPRIORITYR[n];

		}
	}
	break;

	case GICD_ITARGETSR(0) ... GICD_ITARGETSR_LAST: {
		uint32_t n = (offset - GICD_ITARGETSR(0)) >> 2;
		if (n < NR_BANKED_ITARGETSR) {
			return gicd_banked->ITARGETSR[n];
		} else {
			return gicd->ITARGETSR[n];
		}
	}
	break;

	case GICD_ICFGR(0) ... GICD_ICFGR_LAST: {
		uint32_t n = (offset - GICD_ICFGR(0));
		if (n == 1) {
			return gicd_banked->ICFGR;
		} else {
			return gicd->ICFGR[n];
		}
	}
	break;

	case GICD_NSACR(0) ... GICD_NSACR_LAST:
		printf("vgicd: GICD_NSACR read not implemented\n", __func__);
		return HVMM_STATUS_BAD_ACCESS;
		break;

	case GICD_SGIR:
		printf("GICD_SGIR is WO\n");
		return 0;
		break;

	case GICD_CPENDSGIR(0) ... GICD_CPENDSGIR_LAST: {
		return gicd_banked->CPENDSGIR[offset - GICD_CPENDSGIR(0)];
	}
	break;

	case GICD_SPENDSGIR(0) ... GICD_SPENDSGIR_LAST: {
		return gicd_banked->SPENDSGIR[offset - GICD_SPENDSGIR(0)];
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

struct vdev_module vgicd = {
		.name = "vGICD",
		.base = CFG_GIC_BASE_PA | GICD_OFFSET,
		.size = 4096,
		.read = vgicd_read_handler,
		.write = vgicd_write_handler,
};

hvmm_status_t vdev_gicd_init() {
	hvmm_status_t result = HVMM_STATUS_BUSY;

	result = vdev_register(&vgicd);
	ITLinesNumber = GICv2.ITLinesNumber;

	if (result == HVMM_STATUS_SUCCESS) {
		printf("vdev registered:'%s'\n", vgicd.name);
	} else {
		printf("%s: Unable to register vdev:'%s' code=%x\n", __func__,
				vgicd.name, result);
	}

	return result;
}

vdev_module_low_init(vdev_gicd_init);

