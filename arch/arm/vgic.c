#include "vgic.h"
#include "../../drivers/gic-v2.h"
#include <arch/gic_regs.h>

void vgic_init(struct vgic *vgic)
{
    int i;

     struct regs *gicd_regs;

    gicd_regs = &vgic->regs;

    gicd_regs->CTLR     = GICD_READ(GICD_CTLR_OFFSET);
    gicd_regs->TYPER    = GICD_READ(GICD_TYPER_OFFSET);
    gicd_regs->IIDR     = GICD_READ(GICD_IIDR_OFFSET);

    for (i = 0; i < VGICD_NUM_IGROUPR; i++) {
        gicd_regs->IGROUPR[i] = GICD_READ(GICD_IGROUPR(i));
    }

    for (i = 0; i < VGICE_NUM_ISCENABLER; i++) {
        gicd_regs->ISCENABLER[i] = GICD_READ(GICD_ISENABLER(i));
    }

    for (i = 0; i < VGICE_NUM_ISCPENDR; i++) {
        gicd_regs->ISCPENDR[i] = GICD_READ(GICD_ICPENDR(i));
    }

    for (i = 0; i < VGICE_NUM_ISCACTIVER; i++) {
        gicd_regs->ISCACTIVER[i] = GICD_READ(GICD_ISACTIVER(i));
    }

    for (i = 0; i < VGICE_NUM_IPRIORITYR; i++) {
        gicd_regs->IPRIORITYR[i] = GICD_READ(GICD_ISACTIVER(i));
    }

    for (i = 0; i < VGICE_NUM_ITARGETSR; i++) {
        gicd_regs->ITARGETSR[i] = GICD_READ(GICD_ITARGETSR(i));
    }

    for (i = 0; i < VGICE_NUM_ICFGR; i++) {
        gicd_regs->ICFGR[i] = GICD_READ(GICD_ICFGR(i));
    }

    gicd_regs->SGIR = GICD_READ(GICD_SGIR(0));

    for (i = 0; i < VGICD_BANKED_NUM_CPENDSGIR; i++) {
        gicd_regs->CPENDSGIR[i] = GICD_READ(GICD_CPENDSGIR(i));
    }
}

