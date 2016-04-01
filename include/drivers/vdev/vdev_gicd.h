#ifndef __VDEV_GICD_H__
#define __VDEV_GICD_H__

/* Banked Registers Size */
#define NR_BANKED_IPRIORITYR  8
#define NR_BANKED_ITARGETSR   8
#define NR_BANKED_CPENDSGIR   4
#define NR_BANKED_SPENDSGIR   4

/* We assume that ITLinesNumber has maximum number */
#define NR_IRQS             31

#define NR_IGROUPR          (NR_IRQS + 1)
#define NR_ISENABLER        (NR_IRQS + 1)
#define NR_ICENABLER        (NR_IRQS + 1)
#define NR_ISPENDR          (NR_IRQS + 1)
#define NR_ICPENDR          (NR_IRQS + 1)
#define NR_ISACTIVER        (NR_IRQS + 1)
#define NR_ICACTIVER        (NR_IRQS + 1)
#define NR_IPRIORITYR       (8 * (NR_IRQS + 1))
#define NR_ITARGETSR        (8 * (NR_IRQS + 1))
#define NR_ICFGR            (2 * (NR_IRQS + 1))
#define NR_NSACR            32

#define NR_VCPUS			8

struct vgicd {
    uint32_t ctlr;
    uint32_t typer;
    uint32_t iidr;

    uint32_t igroupr0[NR_VCPUS];
    uint32_t igroupr[NR_IGROUPR];

    uint32_t isenabler0[NR_VCPUS];
    uint32_t isenabler[NR_ISENABLER];

    uint32_t icenabler0[NR_VCPUS];
    uint32_t icenabler[NR_ICENABLER];

    uint32_t ispendr0[NR_VCPUS];
    uint32_t ispendr[NR_ISPENDR];

    uint32_t icpendr0[NR_VCPUS];
    uint32_t icpendr[NR_ICPENDR];

    uint32_t isactiver0[NR_VCPUS];
    uint32_t isactiver[NR_ISACTIVER];

    uint32_t icactiver0[NR_VCPUS];
    uint32_t icactiver[NR_ICACTIVER];

    uint32_t ipriorityr0[NR_VCPUS][NR_BANKED_IPRIORITYR];
    uint32_t ipriorityr[NR_IPRIORITYR];

    uint32_t itargetsr0[NR_VCPUS][NR_BANKED_ITARGETSR];
    uint32_t itargetsr[NR_ITARGETSR];

    uint32_t icfgr0[NR_VCPUS];
    uint32_t icfgr[NR_ICFGR];

    uint32_t nsacr[NR_NSACR];			// unused

    uint32_t sgir;

    uint32_t cpendsgir0[NR_VCPUS][NR_BANKED_CPENDSGIR];
    uint32_t spendsgir0[NR_VCPUS][NR_BANKED_SPENDSGIR];
};


#endif /* __VDEV_GICD_H__ */
