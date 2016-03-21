#ifndef __VGIC_H__
#define __VGIC_H__

#include <stdlib.h>

// FIXME(casionwoo) : This TILinesNumber should be modified like GICv2.TILinesNumber as dynamically and also size of virtual registers also should be set dinamically
#define TILinesNumber 4
#define NUM_MAX_VIRQS   160

#define NUM_STATUS_WORDS    (NUM_MAX_VIRQS / 32)
#define VGICD_NUM_ISCPENDR (TILinesNumber + 1)
#define VGICD_NUM_ISCENABLER (TILinesNumber + 1)
#define VGICD_NUM_ISCACTIVER (TILinesNumber + 1)
#define VGICD_NUM_IPRIORITYR (8*(TILinesNumber + 1))
#define VGICD_NUM_ITARGETSR (8*(TILinesNumber + 1))
#define VGICD_NUM_ICFGR (2*(TILinesNumber + 1))
#define VGICD_NUM_SPISR (TILinesNumber + 1)
#define VGICD_NUM_CPENDSGIR (TILinesNumber + 1)
#define VGICD_NUM_SPENDSGIR (TILinesNumber + 1)
#define VGICD_NUM_IGROUPR (TILinesNumber + 1)
#define VGICD_NUM_IENABLER (TILinesNumber + 1)

/* Banked Registers Size */

#define VGICD_BANKED_NUM_IPRIORITYR  8
#define VGICD_BANKED_NUM_ITARGETSR  8
#define VGICD_BANKED_NUM_CPENDSGIR  VGICD_NUM_CPENDSGIR
#define VGICD_BANKED_NUM_SPENDSGIR  VGICD_NUM_SPENDSGIR

struct gicd_regs {
    uint32_t CTLR;                              /*0x000 RW*/
    uint32_t TYPER;                             /*      RO*/
    uint32_t IIDR;                              /*      RO*/

    uint32_t IGROUPR[VGICD_NUM_IGROUPR];        /* 0x080 */
    uint32_t ISENABLER[VGICD_NUM_ISCENABLER];   /* 0x100, ISENABLER/ICENABLER */
    uint32_t ICENABLER[VGICD_NUM_ISCENABLER];   /* 0x100, ISENABLER/ICENABLER */
    uint32_t ISPENDR[VGICD_NUM_ISCPENDR];       /* 0x200, ISPENDR/ICPENDR */
    uint32_t ICPENDR[VGICD_NUM_ISCPENDR];       /* 0x200, ISPENDR/ICPENDR */
    uint32_t ISACTIVER[VGICD_NUM_ISCACTIVER];   /* 0x300, ISACTIVER/ICACTIVER */
    uint32_t ICACTIVER[VGICD_NUM_ISCACTIVER];   /* 0x300, ISACTIVER/ICACTIVER */
    uint32_t IPRIORITYR[VGICD_NUM_IPRIORITYR];  /* 0x400 */
    uint32_t ITARGETSR[VGICD_NUM_ITARGETSR];    /* 0x800 [0]: RO, Otherwise, RW */
    uint32_t ICFGR[VGICD_NUM_ICFGR];            /* 0xC00 */

    /* Cortex-A15 */
    /* 0xD00 GICD_PPISR RO */
    /* 0xD04 ~ 0xD1C GICD_SPISRn RO */

    uint32_t NSACR[64];                         /* 0xE00 */
    uint32_t SGIR;                              /* 0xF00 WO */
};

struct gicd_regs_banked {
    uint32_t IGROUPR;                                  //0
    uint32_t ISENABLER;                                //0
    uint32_t ICENABLER;                                //0
    uint32_t ISPENDR;                                  //0
    uint32_t ICPENDR;                                  //0
    uint32_t ISACTIVER;                                //0
    uint32_t ICACTIVER;                                //0
    uint32_t IPRIORITYR[VGICD_BANKED_NUM_IPRIORITYR];  //0~7
    uint32_t ITARGETSR[VGICD_BANKED_NUM_ITARGETSR];    //0~7
    uint32_t ICFGR;                                    //1
    uint32_t CPENDSGIR[VGICD_BANKED_NUM_CPENDSGIR];    //n
    uint32_t SPENDSGIR[VGICD_BANKED_NUM_SPENDSGIR];    //n
};

struct vgic {
    struct gicd_regs gicd_regs;
};

void vgic_init(struct vgic *vgic);
void gicd_regs_banked_init(struct gicd_regs_banked *regs_banked);

#endif /* __VGIC_H__ */

