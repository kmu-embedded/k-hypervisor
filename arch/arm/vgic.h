#ifndef __VGIC_H__
#define __VGIC_H__

#include <stdlib.h>

/* hard coding for arndale, fastmodel */
#define TILinesNumber 4
#define NUM_MAX_VIRQS   160

#define NUM_STATUS_WORDS    (NUM_MAX_VIRQS / 32)
#define VGICE_NUM_ISCPENDR (TILinesNumber + 1)
#define VGICE_NUM_ISCENABLER (TILinesNumber + 1)
#define VGICE_NUM_ISCACTIVER (TILinesNumber + 1)
#define VGICE_NUM_IPRIORITYR (8*(TILinesNumber + 1))
#define VGICE_NUM_ITARGETSR (8*(TILinesNumber + 1))
#define VGICE_NUM_ICFGR (2*(TILinesNumber + 1))
#define VGICE_NUM_SPISR (TILinesNumber + 1)
#define VGICE_NUM_CPENDSGIR (TILinesNumber + 1)
#define VGICE_NUM_SPENDSGIR (TILinesNumber + 1)
#define VGICD_NUM_IGROUPR (TILinesNumber + 1)
#define VGICD_NUM_IENABLER (TILinesNumber + 1)

/* Banked Registers Size */

#define VGICD_BANKED_NUM_IPRIORITYR  8
#define VGICD_BANKED_NUM_ITARGETSR  8
#define VGICD_BANKED_NUM_CPENDSGIR  VGICE_NUM_CPENDSGIR
#define VGICD_BANKED_NUM_SPENDSGIR  VGICD_NUM_SPENDSGIR

struct regs {
    uint32_t CTLR;
    uint32_t TYPER;
    uint32_t IIDR;

    uint32_t IGROUPR[VGICD_NUM_IGROUPR];
    uint32_t ISCENABLER[VGICE_NUM_ISCENABLER];
    uint32_t ISCPENDR[VGICE_NUM_ISCPENDR];
    uint32_t ISCACTIVER[VGICE_NUM_ISCACTIVER];
    uint32_t IPRIORITYR[VGICE_NUM_IPRIORITYR];
    uint32_t ITARGETSR[VGICE_NUM_ITARGETSR];
    uint32_t ICFGR[VGICE_NUM_ICFGR];

    uint32_t NSACR[64];
    uint32_t SGIR;
    uint32_t CPENDSGIR[VGICE_NUM_CPENDSGIR];
};

struct vgic {
    struct regs regs;
};

void vgic_init(struct vgic *vgic);

#endif /* __VGIC_H__ */
