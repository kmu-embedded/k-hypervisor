#include <core/vm/vgic.h>
#include <stdio.h>
#include <arch/gic_regs.h>
#include "../../drivers/gic-v2.h"

void vgic_init(struct vgic *vgic)
{
    printf("%s START\n", __func__);

    int i = 0;
    struct gicd_regs *gicd_regs = &vgic->gicd_regs;
    struct gicd_regs_banked *gicd_regs_banked = &vgic->gicd_regs_banked;

    gicd_regs->CTLR  = 0;
    gicd_regs->TYPER = GICD_READ(GICD_TYPER_OFFSET);
    gicd_regs->IIDR  = GICD_READ(GICD_IIDR_OFFSET);

    for (i = 0; i < VGICD_NUM_IGROUPR; i++) {
        if (!i) {
            gicd_regs_banked->IGROUPR = 0;
        } else {
            gicd_regs->IGROUPR[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_ISCENABLER; i++) {
        if (!i) {
            gicd_regs_banked->ISENABLER = 0;
            gicd_regs_banked->ICENABLER = 0;
        } else {
            gicd_regs->ISENABLER[i] = 0;
            gicd_regs->ICENABLER[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_ISCPENDR; i++) {
        if (!i) {
            gicd_regs_banked->ISPENDR = 0;
            gicd_regs_banked->ICPENDR = 0;
        } else {
            gicd_regs->ISPENDR[i] = 0;
            gicd_regs->ICPENDR[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_ISCACTIVER; i++) {
        if (!i) {
            gicd_regs_banked->ISACTIVER = 0;
            gicd_regs_banked->ICACTIVER = 0;
        } else {
            gicd_regs->ISACTIVER[i] = 0;
            gicd_regs->ICACTIVER[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_IPRIORITYR; i++) {
        if (i < VGICD_BANKED_NUM_IPRIORITYR) {
            gicd_regs_banked->IPRIORITYR[i] = 0;;
        } else {
            gicd_regs->IPRIORITYR[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_ITARGETSR; i++) {
        if (i < VGICD_BANKED_NUM_ITARGETSR) {
            gicd_regs_banked->ITARGETSR[i] = 0;
        } else {
            gicd_regs->ITARGETSR[i] = 0;
        }
    }

    for (i = 0; i < VGICD_NUM_ICFGR; i++) {
        if (i == 1) {
            gicd_regs_banked->ICFGR = 0;
        } else {
            gicd_regs->ICFGR[i] = 0;
        }
    }

    gicd_regs->SGIR = 0;
    for (i = 0; i < VGICD_BANKED_NUM_CPENDSGIR; i++) {
        gicd_regs_banked->SPENDSGIR[i] = 0;
        gicd_regs_banked->CPENDSGIR[i] = 0;
    }

    printf("%s END\n", __func__);
}
