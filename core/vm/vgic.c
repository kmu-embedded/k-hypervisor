#include <core/vm/vgic.h>
#include <stdio.h>
#include <arch/gic_regs.h>
#include "../../drivers/gic-v2.h"

static uint32_t ITLinesNumber = 0;

void vgic_setup()
{
    ITLinesNumber = GICv2.ITLinesNumber;
}

void vgic_create(struct vgic *vgic)
{
    struct gicd *regs = &vgic->gicd;

    regs->IGROUPR       = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ISENABLER     = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ICENABLER     = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ISPENDR       = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ICPENDR       = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ISACTIVER     = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->ICACTIVER     = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->IPRIORITYR    = malloc(sizeof(uint32_t) * 8 * (ITLinesNumber + 1));
    regs->ITARGETSR     = malloc(sizeof(uint32_t) * 8 * (ITLinesNumber + 1));
    regs->ICFGR         = malloc(sizeof(uint32_t) * 2 * (ITLinesNumber + 1));
    regs->IGROUPR       = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs->NSACR         = malloc(sizeof(uint32_t) * 32);
}

void vgic_init(struct vgic *vgic)
{
    printf("%s START\n", __func__);

    int i = 0;
    struct gicd *gicd = &vgic->gicd;

    gicd->CTLR  = 0;
    gicd->TYPER = GICD_READ(GICD_TYPER_OFFSET);
    gicd->IIDR  = GICD_READ(GICD_IIDR_OFFSET);

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd->IGROUPR[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd->ISENABLER[i] = 0;
        gicd->ICENABLER[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd->ISPENDR[i] = 0;
        gicd->ICPENDR[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd->ISACTIVER[i] = 0;
        gicd->ICACTIVER[i] = 0;
    }

    for (i = 0; i < 8*(ITLinesNumber + 1); i++) {
        gicd->IPRIORITYR[i] = 0;
    }

    for (i = 0; i < 8*(ITLinesNumber + 1); i++) {
        gicd->ITARGETSR[i] = 0;
    }

    for (i = 0; i < 2*(ITLinesNumber + 1); i++) {
        gicd->ICFGR[i] = 0;
    }

    gicd->SGIR = 0;

    printf("%s END\n", __func__);
}

