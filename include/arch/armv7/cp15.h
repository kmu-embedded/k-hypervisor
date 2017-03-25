#ifndef __CP15_H__
#define __CP15_H__

#include "mcrmrc.h"

#define __CP32(cp, n, opc1, m, opc2)    ((n) << 12 | (opc1) << 8 | (m) << 4 | (opc2))
#define CP32(args...)                   __CP32(args)

#define __CP64(cp, opc1, m)    		((opc1) << 8 | (m) << 4)
#define CP64(args...)          		__CP64(args)


//  CP_NAME                		CP#, CRn, OPC1, CRm, OPC2
// CRn == 0, ID registers.
#define MIDR				15, 0, 0, 0, 0
#define CTR				15, 0, 0, 0, 1
#define TMCTR				15, 0, 0, 0, 2
#define TLBTR				15, 0, 0, 0, 3
#define AMIDR0				15, 0, 0, 0, 4
#define MPIDR				15, 0, 0, 0, 5

#ifndef OPTIONAL_REV
#define AMIDR1				15, 0, 0, 0, 6
#else
#define REVIDR				15, 0, 0, 0, 6
#endif

#define AMIDR2				15, 0, 0, 0, 7

#define	ID_PFR0				15, 0, 0, 1, 0
#define	ID_PFR1				15, 0, 0, 1, 1
#define	ID_DFR0				15, 0, 0, 1, 2
#define	ID_AFR0				15, 0, 0, 1, 3
#define	ID_MMFR0			15, 0, 0, 1, 4
#define	ID_MMFR1			15, 0, 0, 1, 5
#define	ID_MMFR2			15, 0, 0, 1, 6
#define	ID_MMFR3			15, 0, 0, 1, 7

#define	ID_ISAR0			15, 0, 0, 2, 0
#define	ID_ISAR1			15, 0, 0, 2, 1
#define	ID_ISAR2			15, 0, 0, 2, 2
#define	ID_ISAR3			15, 0, 0, 2, 3
#define	ID_ISAR4			15, 0, 0, 2, 4
#define	ID_ISAR5			15, 0, 0, 2, 5

#define	CCSIDR				15, 0, 1, 0, 0
#define	CLIDR				15, 0, 1, 0, 1
#define AIDR				15, 0, 1, 0, 7
#define CSSELR				15, 0, 2, 0, 0
#define VPIDR				15, 0, 4, 0, 0
#define VMPIDR				15, 0, 4, 0, 5

// CRn == 1, System control registers.
#define SCTLR				15, 1, 0, 0, 0
#define ACTLR				15, 1, 0, 0, 1
#define CPACR				15, 1, 0, 0, 2
#define	SCR				15, 1, 0, 1, 0
#define SDER				15, 1, 0, 1, 1
#define NSACR				15, 1, 0, 1, 2

#define HSCTLR				15, 1, 4, 0, 0
#define HACTLR				15, 1, 4, 0, 1

#define HCR				15, 1, 4, 1, 0
#define HDCR				15, 1, 4, 1, 1
#define HCPTR				15, 1, 4, 1, 2
#define	HSTR				15, 1, 4, 1, 3
#define HACR				15, 1, 4, 1, 7

// CRn == 2, 3, Memory protection and control registers.
#define TTBR0_32			15, 2, 0, 0, 0
#define TTBR1_32			15, 2, 0, 0, 1

#define TTBR0_64			15, 0, 2
#define TTBR1_64			15, 1, 2

#define TTBCR				15, 2, 0, 0, 2
#define HTCR				15, 2, 4, 0, 2
#define VTCR				15, 2, 4, 1, 2

#define HTTBR				15, 4, 2
#define VTTBR				15, 6, 2

#define DACR				15, 3, 0, 0, 0

// CRn == 5, 6, Memory system fault registers.
#define DFSR				15, 5, 0, 0, 0
#define IFSR				15, 5, 0, 0, 1
#define ADFSR				15, 5, 0, 1, 0
#define AIFSR				15, 5, 0, 1, 1
#define HADFSR				15, 5, 4, 1, 0
#define HAIFSR				15, 5, 4, 1, 1
#define HSR				15, 5, 4, 2, 0

#define DFAR				15, 6, 0, 0, 0
#define IFAR				15, 6, 0, 0, 2

#define HDFAR				15, 6, 4, 0, 0
#define HIFAR				15, 6, 4, 0, 2
#define HPFAR				15, 6, 4, 0, 4
// CRn == 7, Cache maintenance, address translations, miscellaneous.
#define ICALLUIS			15, 7, 0, 1, 0
#define BPIALLIS			15, 7, 0, 1, 6

#define PAR_32				15, 7, 0, 4, 0
#define	PAR_64				15, 0, 7
#define ICIALLU				15, 7, 0, 5, 0
#define ICIMVAU				15, 7, 0, 5, 1
#define CP15ISB				15, 7, 0, 5, 4
#define BPIALL				15, 7, 0, 5, 6
#define BPIMVA				15, 7, 0, 5, 7
#define DCIMVAC				15, 7, 0, 6, 1
#define DCISW				15, 7, 0, 6, 2

#define ATS1CPR				15, 7, 0, 8, 0
#define ATS1CPW				15, 7, 0, 8, 1
#define ATS1CUR				15, 7, 0, 8, 2
#define ATS1CUW				15, 7, 0, 8, 3
#define ATS12NSOPR			15, 7, 0, 8, 4
#define ATS12NSOPW			15, 7, 0, 8, 5
#define ATS1ATS12NSOUR			15, 7, 0, 8, 6
#define ATS12NSOUW			15, 7, 0, 8, 7

#define	DCCMVAC				15, 7, 0, 10, 1
#define DCCSW				15, 7, 0, 10, 2
#define CP15DSB				15, 7, 0, 10, 4
#define CP15DMB				15, 7, 0, 10, 5

#define DCCMVAU				15, 7, 0, 11, 1

#define DCCIMVAC			15, 7, 0, 14, 1
#define DCCISW				15, 7, 0, 14, 2

#define ATS1HR				15, 7, 4, 8, 0
#define ATS1HW				15, 7, 4, 8, 1

// CRn == 8, TLB maintenance operations.
#define TLBIALLIS			15, 8, 0, 3, 0
#define TLBMVAIS			15, 8, 0, 3, 1
#define TLBIASIDIS			15, 8, 0, 3, 2
#define TLBIMVAAIS			15, 8, 0, 3, 3

#define ITLBIALL			15, 8, 0, 5, 0
#define ITLMBIMVA			15, 8, 0, 5, 1
#define ITLBIASID			15, 8, 0, 5, 2

#define DTLBIALL			15, 8, 0, 6, 0
#define DTLBIMVA			15, 8, 0, 6, 1
#define DTLBIASID			15, 8, 0, 6, 2

#define TLBIALL				15, 8, 0, 7, 0
#define TLBIMVA				15, 8, 0, 7, 1
#define TLBIASID			15, 8, 0, 7, 2
#define TLBIASVAA			15, 8, 0, 7, 3

#define TLBIALLHIS			15, 8, 4, 3, 0
#define TLBIMVAHIS			15, 8, 4, 3, 1
#define TLBIALLNSNHIS			15, 8, 4, 3, 4

#define TLBIALLH			15, 8, 4, 7, 0
#define TLBIMVAH			15, 8, 4, 7, 1
#define TLBIALLNSNH			15, 8, 4, 7, 4

// CRm == 0-2, 5-8, CACHE: See Cache and TCM lockdown registers, VMSA on page B4-1753.

// CRn == 9, Reserved for performance monitors and maintenance operation.
#define	PMCR				15, 9, 0, 12, 0
#define PMCNTENSET			15, 9, 0, 12, 1
#define PMCNTENCLR			15, 9, 0, 12, 2
#define PMOVSR				15, 9, 0, 12, 3
#define PMSWINC				15, 9, 0, 12, 4
#define PMSELR				15, 9, 0, 12, 5
#define PMCEID0				15, 9, 0, 12, 6
#define PMCEID1				15, 9, 0, 12, 7

#define PMCCNTR				15, 9, 0, 13, 0
#define PMXEVTYPER			15, 9, 0, 13, 1
#define PMXEVCNTR			15, 9, 0, 13, 2

#define PMUSERENR			15, 9, 0, 14, 0
#define PMINTENSET			15, 9, 0, 14, 1
#define PMINTENCLR			15, 9, 0, 14, 2
#define PMOVSSET			15, 9, 0, 14, 3

// CRm == 12-14, 15, PERFORMANCE MONITOR: See Performance Monitors, functional group on page B3-1500.
// See IMPLEMENTATION DEFINED TLB control operations, VMSA on page B4-1753

// CRn == 10, Memory mapping registers and TLB operations.
#ifndef NOT_LPAE
#define MAIR0				15, 10, 0, 2, 0
#define MAIR1				15, 10, 0, 2, 1
#else
#define PRRR				15, 10, 0, 2, 0
#define NMRR				15, 10, 0, 2, 1
#endif

#define AMAIR0				15, 10, 0, 3, 0
#define AMAIR1				15, 10, 0, 3, 1

#define HMAIR0				15, 10, 4, 2, 0
#define HMAIR1				15, 10, 4, 2, 1
#define HAMAIR0				15, 10, 4, 3, 0
#define HAMAIR1				15, 10, 4, 3, 1

// CRn == 11, Reserved for DMA operations for TCM access
// See DMA support, VMSA on page B4-1754

// CRn == 12, Security Extensions registers, if implemented.
#define VBAR				15, 12, 0, 0, 0
#define MVBAR				15, 12, 0, 0, 1
#define ISR				15, 12, 0, 1, 0

#define HVBAR				15, 12, 4, 0, 0

// CRn == 13, Process, context, and thread ID registers.
#define FCSEIDR				15, 13, 0, 0, 0
#define CONTEXTIDR			15, 13, 0, 0, 1
#define TPIDRURW			15, 13, 0, 0, 2
#define TPIDRURO			15, 13, 0, 0, 3
#define TPIDRPRW			15, 13, 0, 0, 4

#define HTPIDR				15, 13, 4, 0, 2

// CRn == 14, Generic Timer registers, if implemented.
#define CNTFRQ				15, 14, 0, 0, 0
#define CNTPCT				15, 0, 14
#define CNTKCTL				15, 14, 0, 1, 0
#define CNTP_TVAL			15, 14, 0, 2, 0
#define CNTP_CTL			15, 14, 0, 2, 1
#define CNTV_TVAL			15, 14, 0, 3, 0
#define CNTV_CTL			15, 14, 0, 3, 1

#define CNTVCT				15, 1, 14
#define CNTP_CVAL			15, 2, 14
#define CNTV_CVAL			15, 3, 14
#define CNTVOFF				15, 4, 14

#define CNTHCTL				15, 14, 4, 1, 0
#define CNTHP_TVAL			15, 14, 4, 2, 0
#define CNTHP_CTL			15, 14, 4, 2, 1
#define CNTHP_CVAL			15, 6, 14

// CRn == 15, IMPLEMENTATION DEFINED registers.
// See IMPLEMENTATION DEFINED registers, functional group on page B3-1502.
#define CBAR				15, 15,	4, 0, 0

#endif //__CP15_H__
