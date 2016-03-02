#ifndef __GIC_REGS_H__
#define __GIC_REGS_H__

/* Offsets from GIC Base Address */
#define GICD_OFFSET     0x1000
#define GICC_OFFSET     0x2000
#define GICH_OFFSET     0x4000
#define GICV_OFFSET    0x6000

/* Distributor Registers */
#define GICD_CTLR		    0x000			/* v1 ICDDCR */
#define GICD_TYPER		    0x004			/* v1 ICDICTR */
#define GICD_IIDR		    0x008			/* v1 ICDIIDR */
#define GICD_IGROUPR(n)		(0x0080 + ((n) * 4))	/* v1 ICDISER */
#define GICD_ISENABLER(n)	(0x0100 + ((n) * 4))	/* v1 ICDISER */
#define GICD_ICENABLER(n)	(0x0180 + ((n) * 4))	/* v1 ICDICER */
#define GICD_ISPENDR(n)		(0x0200 + ((n) * 4))	/* v1 ICDISPR */
#define GICD_ICPENDR(n)		(0x0280 + ((n) * 4))	/* v1 ICDICPR */
#define GICD_ICACTIVER(n)	(0x0380 + ((n) * 4))	/* v1 ICDABR */
#define GICD_IPRIORITYR(n)	(0x0400 + ((n) * 4))	/* v1 ICDIPR */
#define GICD_ITARGETSR(n)	(0x0800 + ((n) * 4))	/* v1 ICDIPTR */
#define GICD_ICFGR(n)		(0x0C00 + ((n) * 4))	/* v1 ICDICFR */
#define GICD_SGIR(n)		(0x0F00 + ((n) * 4))	/* v1 ICDSGIR */

/* CPU Registers */
#define GICC_CTLR		    0x0000			/* v1 ICCICR */
#define GICC_PMR		    0x0004			/* v1 ICCPMR */
#define GICC_BPR		    0x0008			/* v1 ICCBPR */
#define GICC_IAR		    0x000C			/* v1 ICCIAR */
#define GICC_EOIR		    0x0010			/* v1 ICCEOIR */
#define GICC_RPR		    0x0014			/* v1 ICCRPR */
#define GICC_HPPIR		    0x0018			/* v1 ICCHPIR */
#define GICC_ABPR		    0x001C			/* v1 ICCABPR */
#define GICC_IIDR		    0x00FC			/* v1 ICCIIDR*/
#define GICC_DIR            0x1000


/* Virtual Interface Control */
#define GICH_HCR    (0x00/4)
#define GICH_VTR    (0x04/4)
#define GICH_VMCR   (0x08/4)
#define GICH_MISR   (0x10/4)
#define GICH_EISR0  (0x20/4)
#define GICH_EISR1  (0x24/4)
#define GICH_ELSR0  (0x30/4)
#define GICH_ELSR1  (0x34/4)
#define GICH_APR    (0xF0/4)
#define GICH_LR     (0x100/4)    /* LR0 ~ LRn, n:GICH_VTR.ListRegs */

/* Distributor Register Fields */
#define GICD_CTLR_ENABLE    0x1
#define GICD_TYPE_LINES_MASK    0x01f
#define GICD_TYPE_CPUS_MASK    0x0e0
#define GICD_TYPE_CPUS_SHIFT    5

/* Software Generated Interrupt Fields */
#define GICD_SGIR_TARGET_LIST_FILTER_MASK   (0x3<<24)
#define GICD_SGIR_TARGET_LIST   (0x0<<24)
#define GICD_SGIR_TARGET_OTHER  (0x1<<24)
#define GICD_SGIR_TARGET_SELF  (0x2<<24)

#define GICD_SGIR_CPU_TARGET_LIST_OFFSET    16
#define GICD_SGIR_CPU_TARGET_LIST_MASK  \
    (0xFF<<GICD_SGIR_CPU_TARGET_LIST_OFFSET)
#define GICD_SGIR_SGI_INT_ID_MASK   0xF

/* CPU Interface Register Fields */
#define GICC_CTL_ENABLE     0x1
#define GICC_CTL_EOI        (0x1 << 9)
#define GICC_IAR_MASK    0x03ff

/* Virtual Interface Control */
#define GICH_HCR_EN             0x1
#define GICH_HCR_NPIE           (0x1 << 3)
#define GICH_HCR_LRENPIE        (0x1 << 2)
#define GICH_HCR_UIE            (0x1 << 1)
#define GICH_VTR_PRIBITS_SHIFT  29
#define GICH_VTR_PRIBITS_MASK   (0x7 << GICH_VTR_PRIBITS_SHIFT)
#define GICH_VTR_PREBITS_SHIFT  26
#define GICH_VTR_PREBITS_MASK   (0x7 << GICH_VTR_PREBITS_SHIFT)
#define GICH_VTR_LISTREGS_MASK  0x3f

#define GICH_LR_VIRTUALID_SHIFT     0
#define GICH_LR_VIRTUALID_MASK      (0x3ff << GICH_LR_VIRTUALID_SHIFT)
#define GICH_LR_PHYSICALID_SHIFT    10
#define GICH_LR_PHYSICALID_MASK     (0x3ff << GICH_LR_PHYSICALID_SHIFT)
#define GICH_LR_CPUID_SHIFT         10
#define GICH_LR_CPUID_MASK          (0x7 << GICH_LR_CPUID_SHIFT)
#define GICH_LR_EOI_SHIFT           19
#define GICH_LR_EOI_MASK            (0x1 << GICH_LR_EOI_SHIFT)
#define GICH_LR_EOI                 (0x1 << GICH_LR_EOI_SHIFT)
#define GICH_LR_PRIORITY_SHIFT      23
#define GICH_LR_PRIORITY_MASK       (0x1f << GICH_LR_PRIORITY_SHIFT)
#define GICH_LR_STATE_SHIFT     28
#define GICH_LR_STATE_MASK      (0x3 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_INACTIVE          (0x0 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_PENDING           (0x1 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_ACTIVE            (0x2 << GICH_LR_STATE_SHIFT)
#define GICH_LR_STATE_PENDING_ACTIVE    (0x3 << GICH_LR_STATE_SHIFT)
#define GICH_LR_GRP1_SHIFT      30
#define GICH_LR_GRP1_MASK       (0x1 << GICH_LR_GRP1_SHIFT)
#define GICH_LR_GRP1            (0x1 << GICH_LR_GRP1_SHIFT)
#define GICH_LR_HW_SHIFT      31
#define GICH_LR_HW_MASK       (0x1 << GICH_LR_HW_SHIFT)
#define GICH_LR_HW            (0x1 << GICH_LR_HW_SHIFT)

#define GICH_MISR_EOI           (1)
#define GICH_MISR_U_SHIFT       (1)
#define GICH_MISR_U             (1 << GICH_MISR_U_SHIFT)
#define GICH_MISR_NP_SHIFT      (3)
#define GICH_MISR_NP            (1 << GICH_MISR_NP_SHIFT)
#endif
