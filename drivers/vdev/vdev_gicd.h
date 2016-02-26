#ifndef __VDEV___GICD_H__
#define __VDEV___GICD_H__

/* Distributor */
#define __GICD_CTLR   0x000
#define __GICD_TYPER  (0x004/4)
#define __GICD_IIDR   (0x008/4)
#define __GICD_IGROUPR    (0x080/4)
#define __GICD_ISENABLER    (0x100/4)
#define __GICD_ICENABLER    (0x180/4)
#define __GICD_ISPENDR    (0x200/4)
#define __GICD_ICPENDR    (0x280/4)
#define __GICD_IPRIORITYR    (0x400/4)
#define __GICD_ITARGETSR    (0x800/4)
#define __GICD_ICFGR    (0xC00/4)

#define __GICD_SGIR   (0xF00/4)
#define __GICD_CPENDSGIR  (0xF10/4)
#define __GICD_SPENDSGIR  (0xF20/4)

/* Distributor offset */
#define __GICD_OFFSET_CTLR   0x000
#define __GICD_OFFSET_TYPER  0x004
#define __GICD_OFFSET_IIDR   0x008
#define __GICD_OFFSET_IGROUPR    0x080
#define __GICD_OFFSET_ISENABLER    0x100
#define __GICD_OFFSET_ICENABLER    0x180
#define __GICD_OFFSET_ISPENDR    0x200
#define __GICD_OFFSET_ICPENDR    0x280
#define __GICD_OFFSET_ISCACTIVER    0x300
#define __GICD_OFFSET_IPRIORITYR    0x400
#define __GICD_OFFSET_ITARGETSR    0x800
#define __GICD_OFFSET_ICFGR    0xC00
#define __GICD_OFFSET_CPENDGIR    0xF10

#define __GICD_OFFSET_SGIR    0xF00
#define __GICD_OFFSET_CPENDSGIR   0xF10
#define __GICD_OFFSET_SPENDSGIR   0xF20

#endif //__VDEV___GICD_H__
