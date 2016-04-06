#include <stdio.h>
#include <vdev.h>
#include <io.h>
#include <core/scheduler.h>
#include <irq-chip.h>

//In rtsm this 37 is for serial 0
#define PL01x_IRQ_NUM   37

// FIXME(casionwoo) : This owner should be changed to variable not 'define' And this OWNER means VM
#define PL01X_OWNER     1

#define UART_BASE 0x1C090000

#define UART_ADDR(x)        (UART_BASE + x)

#define UARTDR              0x000
#define UARTRSR_UARTECR     0x004
#define UARTFR              0x018
#define UARTILPR            0x020
#define UARTIBRD            0x024
#define UARTFBRD            0x028
#define UARTLCR_H           0x02C
#define UARTCR              0x030
#define UARTIFLS            0x034
#define UARTMSC             0x038
#define UARTRIS             0x03C
#define UARTMIS             0x040
#define UARTICR             0x044
#define UARTDMACR           0x048
#define UARTPERIPHID(x)     (0xFE0 + (4 * x))
#define UARTCELLID(x)       (0xFF0 + (4 * x))

extern struct virq_chip *virq_hw;

int32_t vuart_read(void *pdata, uint32_t offset);
int32_t vuart_write(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vuart_create(void **pdata);

struct pl01x {
	uint32_t uartdr;
	uint32_t uartrsr_uartecr;
	uint32_t uartfr;
	uint32_t uartilpr;
	uint32_t uartibrd;
	uint32_t uartfbrd;
	uint32_t uartlcr_h;
	uint32_t uartcr;
	uint32_t uartifls;
	uint32_t uartmsc;
	uint32_t uartris;
	uint32_t uartmis;
	uint32_t uarticr;
	uint32_t uartdmacr;
	uint32_t uartperiphid[4];
	uint32_t uartcellid[4];
};

struct vdev_module pl01x_vuart = {
		.name   = "vdev_pl01x",
		.base   = UART_BASE,
		.size   = 4096,
        .read   = vuart_read,
		.write  = vuart_write,
		.create = vuart_create,
};

static void vdev_pl01x_irq_handler(int irq, void *regs, void *pdata)
{
    struct vcpu *vcpu = vcpu_find(PL01X_OWNER);
    virq_hw->forward_irq(vcpu->vcpuid, irq, irq, INJECT_SW);
}

#include <stdlib.h>
int32_t vuart_create(void **pdata)
{
	struct pl01x *vuart = malloc(sizeof(struct pl01x));

	*pdata = vuart;
	return 0;
}

int32_t vuart_write(void *pdata, uint32_t offset, uint32_t *addr)
{
	struct pl01x *vuart = pdata;
    struct vcpu *vcpu = get_current_vcpu();

	switch (offset) {
	case UARTDR:
//		printf("%s UARTDR\n", __func__);
		vuart->uartdr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
    		writel(vuart->uartdr, UART_ADDR(UARTDR));
		break;

	case UARTRSR_UARTECR:
//		printf("%s UARTRSR_UARTECR\n", __func__);
		vuart->uartrsr_uartecr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartrsr_uartecr, UART_ADDR(UARTRSR_UARTECR));
		break;

	case UARTILPR:
//		printf("%s UARTILPR\n", __func__);
		vuart->uartilpr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartilpr, UART_ADDR(UARTILPR));
		break;

	case UARTIBRD:
//		printf("%s UARTBRD, data : %x\n", __func__, readl(addr));
		vuart->uartibrd = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartibrd, UART_ADDR(UARTIBRD));
		break;

	case UARTFBRD:
//		printf("%s UARTFBRD\n", __func__);
		vuart->uartfbrd = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartfbrd, UART_ADDR(UARTFBRD));
		break;

	case UARTLCR_H:
//		printf("%s UARTLCR_H\n", __func__);
		vuart->uartlcr_h = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartlcr_h, UART_ADDR(UARTLCR_H));
		break;

	case UARTCR:
//		printf("%s UARTCR, data : %x\n", __func__, readl(addr));
		vuart->uartcr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartcr, UART_ADDR(UARTCR));
		break;

	case UARTIFLS:
//		printf("%s UARTIFLS\n", __func__);
		vuart->uartifls = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartifls, UART_ADDR(UARTIFLS));
		break;

	case UARTMSC:
//		printf("%s UARTMSC\n", __func__);
		vuart->uartmsc = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartmsc, UART_ADDR(UARTMSC));
		break;

	case UARTICR:
//		printf("%s UARTICR\n", __func__);
		vuart->uarticr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uarticr, UART_ADDR(UARTICR));
		break;

	case UARTDMACR:
//		printf("%s UARTDMACR\n", __func__);
		vuart->uartdmacr = readl(addr);
        if (vcpu->vcpuid == PL01X_OWNER)
		    writel(vuart->uartdmacr, UART_ADDR(UARTDMACR));
		break;

	default:
		printf("%s ERROR!! offset: %x\n", __func__, offset, readl(addr));
		writel(readl(addr), UART_BASE + offset);
    }
	return 0;
}

int32_t vuart_read(void *pdata, uint32_t offset)
{
	struct pl01x *vuart = pdata;
    struct vcpu *vcpu = get_current_vcpu();

	switch (offset)
    {
	case UARTDR:
//		printf("%s UARTDR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
            return readl(UART_ADDR(UARTDR));
        return vuart->uartdr;

	case UARTRSR_UARTECR:
//		printf("%s UARTRSR_UARTECR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTRSR_UARTECR));
        return vuart->uartrsr_uartecr;

	case UARTFR:
//		printf("%s UARTFR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTFR));
        return vuart->uartfr;

	case UARTILPR:
//		printf("%s UARTILPR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTILPR));
        return vuart->uartilpr;

	case UARTIBRD:
//		printf("%s UARTIBRD\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTIBRD));
        return vuart->uartibrd;

	case UARTFBRD:
//		printf("%s UARTFBRD\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTFBRD));
        return vuart->uartfbrd;

	case UARTLCR_H:
//		printf("%s UARTLCR_H\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTLCR_H));
        return vuart->uartlcr_h;

	case UARTCR:
//		printf("%s UARTCR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTCR));
        return vuart->uartcr;

	case UARTIFLS:
//		printf("%s UARTIFLS\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTIFLS));
        return vuart->uartifls;

	case UARTMSC:
//		printf("%s UARTMSC\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTMSC));
        return vuart->uartmsc;

	case UARTRIS:
//		printf("%s UARTRIS\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTRIS));
        return vuart->uartris;

	case UARTMIS:
//		printf("%s UARTMIS\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTMIS));
        return vuart->uartmis;

	case UARTDMACR:
//		printf("%s UARTDMACR\n", __func__);
        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTDMACR));
        return vuart->uartdmacr;

	case UARTPERIPHID(0) ... UARTPERIPHID(3): {
        int index = (offset - UARTPERIPHID(0)) >> 2;
//		printf("%s UARTPERIPHID[%d]\n", __func__, index);

        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTPERIPHID(index)));
        return vuart->uartperiphid[index];
    }

	case UARTCELLID(0) ... UARTCELLID(3): {
        int index = (offset - UARTCELLID(0)) >> 2;
//		printf("%s UARTCELLID[%d]\n", __func__, index);

        if (vcpu->vcpuid == PL01X_OWNER)
		    return readl(UART_ADDR(UARTCELLID(index)));
        return vuart->uartcellid[index];
    }

	default:
		printf("%s ERROR!! offset: %x\n", __func__, offset);
		return readl(UART_BASE + offset);
    }
	return 0;
}

hvmm_status_t vdev_pl01x_init() {
	hvmm_status_t result = HVMM_STATUS_BUSY;

    // For trap
	vdev_register(&pl01x_vuart);

    // For irq
    vdev_irq_handler_register(PL01x_IRQ_NUM, vdev_pl01x_irq_handler);
	printf("vdev registered:'%s'\n", pl01x_vuart.name);

	return result;
}

vdev_module_init(vdev_pl01x_init);
