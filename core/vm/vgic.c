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
    struct gicd_regs *regs = &vgic->gicd_regs;

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
    struct gicd_regs *gicd_regs = &vgic->gicd_regs;

    gicd_regs->CTLR  = 0;
    gicd_regs->TYPER = GICD_READ(GICD_TYPER_OFFSET);
    gicd_regs->IIDR  = GICD_READ(GICD_IIDR_OFFSET);

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd_regs->IGROUPR[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd_regs->ISENABLER[i] = 0;
        gicd_regs->ICENABLER[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd_regs->ISPENDR[i] = 0;
        gicd_regs->ICPENDR[i] = 0;
    }

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd_regs->ISACTIVER[i] = 0;
        gicd_regs->ICACTIVER[i] = 0;
    }

    for (i = 0; i < 8*(ITLinesNumber + 1); i++) {
        gicd_regs->IPRIORITYR[i] = 0;
    }

    for (i = 0; i < 8*(ITLinesNumber + 1); i++) {
        gicd_regs->ITARGETSR[i] = 0;
    }

    for (i = 0; i < 2*(ITLinesNumber + 1); i++) {
        gicd_regs->ICFGR[i] = 0;
    }

    gicd_regs->SGIR = 0;

    printf("%s END\n", __func__);
}

void gicd_banked_regs_create(struct gicd_banked_regs *regs_banked)
{
    regs_banked->IPRIORITYR = malloc(sizeof(uint32_t) * VGICD_BANKED_NUM_IPRIORITYR);
    regs_banked->ITARGETSR  = malloc(sizeof(uint32_t) * VGICD_BANKED_NUM_ITARGETSR);
    regs_banked->CPENDSGIR  = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
    regs_banked->SPENDSGIR  = malloc(sizeof(uint32_t) * (ITLinesNumber + 1));
}

void gicd_banked_regs_init(struct gicd_banked_regs *gicd_banked_regs)
{
    int i = 0;

    gicd_banked_regs->IGROUPR = 0;
    gicd_banked_regs->ISENABLER = 0;
    gicd_banked_regs->ICENABLER = 0;
    gicd_banked_regs->ISPENDR = 0;
    gicd_banked_regs->ICPENDR = 0;
    gicd_banked_regs->ISACTIVER = 0;
    gicd_banked_regs->ICACTIVER = 0;

    for (i = 0; i < VGICD_BANKED_NUM_IPRIORITYR; i++) {
        gicd_banked_regs->IPRIORITYR[i] = 0;;
    }

    for (i = 0; i < VGICD_BANKED_NUM_ITARGETSR; i++) {
        gicd_banked_regs->ITARGETSR[i] = 0;
    }

    gicd_banked_regs->ICFGR = 0;

    for (i = 0; i < (ITLinesNumber + 1); i++) {
        gicd_banked_regs->SPENDSGIR[i] = 0;
        gicd_banked_regs->CPENDSGIR[i] = 0;
    }
}
