#ifndef __HSR_H__
#define __HSR_H__

/* Define bits of HSR */
#define HSR_EC                      26
#define HSR_IL                      25
#define HSR_ISS                     0

/* HSR Exception Class. */
#define EC_UNK                      0x00
#define EC_WFI_WFE                  0x01
#define EC_MCR_MRC_CP15             0x03
#define EC_MCRR_MRRC_CP15           0x04
#define EC_MCR_MRC_CP14             0x05
#define EC_LDC_STC_CP14             0x06
#define EC_HCRTR_CP0_CP13           0x07
#define EC_MRC_VMRS_CP10            0x08
#define EC_BXJ                      0x0A
#define EC_MRRC_CP14                0x0C
#define EC_SVC                      0x11
#define EC_HVC                      0x12
#define EC_SMC                      0x13
#define EC_PABT_FROM_GUEST          0x20
#define EC_PABT_FROM_HYP            0x21
#define EC_DABT_FROM_GUEST          0x24
#define EC_DABT_FROM_HYP            0x25

#define IL_ARM                      1
#define IL_THUMB                    0

#define EC_BIT                      (0b111111)
#define IL_BIT                      (0b1)
#define ISS_BIT                     ~((EC_BIT << HSR_EC) | (IL_BIT << HSR_IL))

#define SET_BIT(x)                  (1 << x)
#define IS_EC_ZERO(x)               (x & (SET_BIT(31) | SET_BIT(30))) >> 30

#define EC(x)                       (x >> HSR_EC)
#define IL(x)                       (x & (1 << HSR_IL)) >> HSR_IL
#define ISS(x)                      (x & ISS_BIT)

/* Define bits of ISS (HSR[24:0]) */
#define ISS_CV                      24
#define ISS_COND                    20

#define CV_BIT                      (0b1)
#define COND_BIT                    (0b1111)

// ifdef TRAP_INSTRUCTION
#define CV(x)                       (x >> ISS_CV)
#define COND(x)                     (x & (COND_BIT << ISS_COND)) >> ISS_COND


// if EC == 0x20 ~ 0x25
#define ISS_ISV                     24
#define ISS_SAS                     22
#define ISS_SSE                     21
//bit[20] in ISS, is reserved.
#define ISS_SRT                     16
//bit[15:10] in ISS, is reserved.
#define ISS_EA                      9
#define ISS_CM                      8
#define ISS_S1PTW                   7
#define ISS_WNR                     6
#define ISS_FSC                     0
#define ISS_DFSC                    ISS_FSC
#define ISS_IFSC                    ISS_FSC

#define ISV_BIT                     (0b1)
#define SAS_BIT                     (0b11)
#define SSE_BIT                     (0b1)
#define SRT_BIT                     (0b1111)
#define EA_BIT                      (0b1)
#define CM_BIT                      (0b1)
#define S1PTW_BIT                   (0b1)
#define WNR_BIT                     (0b1)
#define FSC_BIT                     (0b11111)
#define DFSC_BIT                    FSC_BIT
#define IFSC_BIT                    FSC_BIT

//#define FSC(x)                      (x & FSC_BIT)
#define DFSC(x)                     (x & DFSC_BIT)
#define IFSC(x)                     (x & IFSC_BIT)
#define SRT(x)                      (x & (SRT_BIT << ISS_SRT)) >> ISS_SRT
#define WNR(x)                      (x & (WNR_BIT << ISS_WNR)) >> ISS_WNR

/* Long-descriptor format FSR decodings, x means level */
#define TRANS_FAULT(x)              (0x04 + x)
#define ACCESS_FAULT(x)             (0x08 + x)
#define PERM_FAULT(x)               (0x0C + x)
#define SYNC_ABORT                  (0x10)
#define SYNC_PERORR                 (0x18)
#define ASYNC_ABORT                 (0x11)
#define ASYNC_PERORR                (0x19)
#define ABORT_ON_TABLE_WALK(x)      (0x14 + x)
#define PERORR_ON_TABLE_WALK(x)     (0x1C + x)
#define ALINGMENT_FAULT             (0x21)
#define DEBUG_EVENT                 (0x22)
#define TLB_CONFLICT                (0x30)
/* LOCKDOWN and COPROCESSOR ABORT is IMPLEMENTATION DEFINED */
#define LOCKDOWN                    (0x34)
#define COPROCESSOR_ABORT           (0x3A)
#define DOMAIN_FAULT(x)             (0x3C + x)

#define HPFAR_FIPA                  4
#define IPA_SHIFT                   (12 - HPFAR_FIPA)

// TODO(wonseok): PAGE_OFFSET_MASK will be moved to xxx.
#define PAGE_OFFSET_MASK            0x00000FFF

#endif
