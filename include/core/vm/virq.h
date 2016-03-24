#ifndef __VIRQ_H__
#define __VIRQ_H__

#include <stdlib.h>

/* Banked Registers Size */
#define NR_BANKED_IPRIORITYR  8
#define NR_BANKED_ITARGETSR   8
#define NR_BANKED_CPENDSGIR   4
#define NR_BANKED_SPENDSGIR   4

/* We assume that ITLinesNumber has maximum number */
#define MAX_ITLinesNumber             31

#define NR_IGROUPR          (MAX_ITLinesNumber + 1)
#define NR_ISENABLER        (MAX_ITLinesNumber + 1)
#define NR_ICENABLER        (MAX_ITLinesNumber + 1)
#define NR_ISPENDR          (MAX_ITLinesNumber + 1)
#define NR_ICPENDR          (MAX_ITLinesNumber + 1)
#define NR_ISACTIVER        (MAX_ITLinesNumber + 1)
#define NR_ICACTIVER        (MAX_ITLinesNumber + 1)
#define NR_IPRIORITYR       (8 * (MAX_ITLinesNumber + 1))
#define NR_ITARGETSR        (8 * (MAX_ITLinesNumber + 1))
#define NR_ICFGR            (2 * (MAX_ITLinesNumber + 1))
#define NR_NSACR            32

// Per VM
struct virq {
    uint32_t CTLR;
    uint32_t TYPER;
    uint32_t IIDR;

    uint32_t IGROUPR[NR_IGROUPR];
    uint32_t ISENABLER[NR_ISENABLER];
    uint32_t ICENABLER[NR_ICENABLER];
    uint32_t ISPENDR[NR_ISPENDR];
    uint32_t ICPENDR[NR_ICPENDR];
    uint32_t ISACTIVER[NR_ISACTIVER];
    uint32_t ICACTIVER[NR_ICACTIVER];
    uint32_t IPRIORITYR[NR_IPRIORITYR];
    uint32_t ITARGETSR[NR_ITARGETSR];
    uint32_t ICFGR[NR_ICFGR];
    uint32_t NSACR[NR_NSACR];
    uint32_t SGIR;
};

// Per Core
struct banked_virq {
    uint32_t IGROUPR;
    uint32_t ISENABLER;
    uint32_t ICENABLER;
    uint32_t ISPENDR;
    uint32_t ICPENDR;
    uint32_t ISACTIVER;
    uint32_t ICACTIVER;
    uint32_t IPRIORITYR[NR_BANKED_IPRIORITYR];
    uint32_t ITARGETSR[NR_BANKED_ITARGETSR];
    uint32_t ICFGR;
    uint32_t CPENDSGIR[NR_BANKED_CPENDSGIR];
    uint32_t SPENDSGIR[NR_BANKED_SPENDSGIR];


};

void vgicd_init(struct virq *gicd);

#endif /* __VGIC_H__ */

