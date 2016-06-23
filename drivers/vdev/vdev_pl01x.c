#include <stdio.h>
#include <vdev.h>
#include <io.h>
#include <core/scheduler.h>
#include <irq-chip.h>
#include <string.h>
#include <arch/irq.h>
#include <core/vm/vm.h>

//In rtsm this 37 is for serial 0
#define PL01x_IRQ_NUM   37
#define PROMPT_MSG  "[vm %d] "

static char prompt[32];
static int owner_id = 0;

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

extern struct virq_hw *virq_hw;

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

static irq_return_t vdev_pl01x_irq_handler(int irq, void *regs, void *pdata)
{
    if (irq != PL01x_IRQ_NUM) {
        printf("Uncorrect irq nuber\n");
        return UNEXCEPTED_IRQ;
    }

    int i;
    struct vmcb *vm = vm_find(owner_id);

    for (i = 0; i < vm->num_vcpus; i++) {
        struct vcpu *vcpu = vm->vcpu[i];
        virq_hw->forward_irq(vcpu, PL01x_IRQ_NUM, PL01x_IRQ_NUM, INJECT_SW);
    }

    return VM_IRQ;
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
    case UARTDR: {
        vuart->uartdr = readl(addr);
        if (vcpu->vmid == owner_id) {
            writel(vuart->uartdr, UART_ADDR(UARTDR));
            if (vuart->uartdr == 13) {
                int i;
                for (i = 0; i < sizeof(prompt) / sizeof(char); i++) {
                    writel(prompt[i], UART_ADDR(UARTDR));
                }
            }
        }
        break;
    }

    case UARTRSR_UARTECR:
        vuart->uartrsr_uartecr = readl(addr);
        writel(vuart->uartrsr_uartecr, UART_ADDR(UARTRSR_UARTECR));
        break;

    case UARTILPR:
        vuart->uartilpr = readl(addr);
        writel(vuart->uartilpr, UART_ADDR(UARTILPR));
        break;

    case UARTIBRD:
        vuart->uartibrd = readl(addr);
        writel(vuart->uartibrd, UART_ADDR(UARTIBRD));
        break;

    case UARTFBRD:
        writel(readl(addr), UART_ADDR(UARTFBRD));
        break;

    case UARTLCR_H:
        writel(readl(addr), UART_ADDR(UARTLCR_H));
        break;

    case UARTCR:
        vuart->uartcr = readl(addr);
        writel(vuart->uartcr, UART_ADDR(UARTCR));
        break;

    case UARTIFLS:
        writel(readl(addr), UART_ADDR(UARTIFLS));
        break;

    case UARTMSC:
        vuart->uartmsc = readl(addr);
        if (vuart->uartmsc == 0x70 && vcpu->vmid != owner_id) {
            virq_hw->forward_irq(vcpu, PL01x_IRQ_NUM, PL01x_IRQ_NUM, INJECT_SW);
        }
        writel(vuart->uartmsc, UART_ADDR(UARTMSC));

        break;

    case UARTICR:
        writel(readl(addr), UART_ADDR(UARTICR));
        break;

    case UARTDMACR:
        writel(readl(addr), UART_ADDR(UARTDMACR));
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

    switch (offset) {
    case UARTDR: {
        uint32_t data = readl(UART_ADDR(UARTDR));

        if (vcpu->vmid == owner_id) {
            // TODO(casionwoo) : When data is special key(Ctrl + y), Change the OWNER
            if (data == 25) {
                printf("OWNER FROM vm[%d] to ", owner_id);
                owner_id = (owner_id + 1) % NUM_GUESTS_STATIC;
                printf("vm[%d] \n", owner_id);

                memset(prompt, 0, 32);
                sprintf(prompt, PROMPT_MSG, owner_id);
            }
            return data;
        }
        return vuart->uartdr;
    }
    case UARTRSR_UARTECR:
        return readl(UART_ADDR(UARTRSR_UARTECR));

    case UARTFR:
        return readl(UART_ADDR(UARTFR));

    case UARTILPR:
        return readl(UART_ADDR(UARTILPR));

    case UARTIBRD:
        return readl(UART_ADDR(UARTIBRD));

    case UARTFBRD:
        return readl(UART_ADDR(UARTFBRD));

    case UARTLCR_H:
        return readl(UART_ADDR(UARTLCR_H));

    case UARTCR:
        return readl(UART_ADDR(UARTCR));

    case UARTIFLS:
        return readl(UART_ADDR(UARTIFLS));

    case UARTMSC:
        return readl(UART_ADDR(UARTMSC));

    case UARTRIS:
        return readl(UART_ADDR(UARTRIS));

    case UARTMIS:
        return readl(UART_ADDR(UARTMIS));

    case UARTDMACR:
        return readl(UART_ADDR(UARTDMACR));

    case UARTPERIPHID(0) ... UARTPERIPHID(3): {
        int index = (offset - UARTPERIPHID(0)) >> 2;
        return readl(UART_ADDR(UARTPERIPHID(index)));
    }

    case UARTCELLID(0) ... UARTCELLID(3): {
        int index = (offset - UARTCELLID(0)) >> 2;
        return readl(UART_ADDR(UARTCELLID(index)));
    }

    default:
        printf("%s ERROR!! offset: %x\n", __func__, offset);
        return readl(UART_BASE + offset);
    }
    return 0;
}

hvmm_status_t vdev_pl01x_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    memset(prompt, 0, 32);
    sprintf(prompt, PROMPT_MSG, owner_id);

    // For trap
    vdev_register(&pl01x_vuart);

    // For irq
    register_irq_handler(PL01x_IRQ_NUM, vdev_pl01x_irq_handler, IRQ_LEVEL_SENSITIVE);
    printf("vdev registered:'%s'\n", pl01x_vuart.name);

    return result;
}

vdev_module_init(vdev_pl01x_init);
