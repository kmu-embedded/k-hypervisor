#include <core/vm/vgic.h>
#include <stdio.h>
#include <arch/gic_regs.h>
#include "../../drivers/gic-v2.h"

void vgic_init(struct vgic *vgic)
{
    printf("%s START\n", __func__);

    int i = 0;
    struct gicd_regs *gicd_regs = &vgic->gicd_regs;

    gicd_regs->CTLR  = 0;
    gicd_regs->TYPER = GICD_READ(GICD_TYPER_OFFSET);
    gicd_regs->IIDR  = GICD_READ(GICD_IIDR_OFFSET);

    for (i = 0; i < VGICD_NUM_IGROUPR; i++) {
        gicd_regs->IGROUPR[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_ISCENABLER; i++) {
        gicd_regs->ISENABLER[i] = 0;
        gicd_regs->ICENABLER[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_ISCPENDR; i++) {
        gicd_regs->ISPENDR[i] = 0;
        gicd_regs->ICPENDR[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_ISCACTIVER; i++) {
        gicd_regs->ISACTIVER[i] = 0;
        gicd_regs->ICACTIVER[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_IPRIORITYR; i++) {
        gicd_regs->IPRIORITYR[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_ITARGETSR; i++) {
        gicd_regs->ITARGETSR[i] = 0;
    }

    for (i = 0; i < VGICD_NUM_ICFGR; i++) {
        gicd_regs->ICFGR[i] = 0;
    }

    gicd_regs->SGIR = 0;

    printf("%s END\n", __func__);
}

void gicd_regs_banked_init(struct gicd_regs_banked *gicd_regs_banked)
{
    int i = 0;

    gicd_regs_banked->IGROUPR = 0;
    gicd_regs_banked->ISENABLER = 0;
    gicd_regs_banked->ICENABLER = 0;
    gicd_regs_banked->ISPENDR = 0;
    gicd_regs_banked->ICPENDR = 0;
    gicd_regs_banked->ISACTIVER = 0;
    gicd_regs_banked->ICACTIVER = 0;

    for (i = 0; i < VGICD_BANKED_NUM_IPRIORITYR; i++) {
        gicd_regs_banked->IPRIORITYR[i] = 0;;
    }

    for (i = 0; i < VGICD_BANKED_NUM_ITARGETSR; i++) {
        gicd_regs_banked->ITARGETSR[i] = 0;
    }

    gicd_regs_banked->ICFGR = 0;

    for (i = 0; i < VGICD_BANKED_NUM_CPENDSGIR; i++) {
        gicd_regs_banked->SPENDSGIR[i] = 0;
        gicd_regs_banked->CPENDSGIR[i] = 0;
    }
}
