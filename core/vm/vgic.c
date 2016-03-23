#include <core/vm/vgic.h>
#include <arch/gic_regs.h>
#include <string.h>
#include "../../drivers/gic-v2.h"

void vgic_init(struct vgic *vgic)
{
    struct gicd *gicd = &vgic->gicd;

    memset(vgic, 0, sizeof(struct vgic));

    gicd->TYPER = GICD_READ(GICD_TYPER_OFFSET);
    gicd->IIDR  = GICD_READ(GICD_IIDR_OFFSET);
}

