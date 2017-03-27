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

// x == iss only.
#define CV(x)                       (x >> ISS_CV)
#define COND(x)                     (x & (COND_BIT << ISS_COND)) >> ISS_COND

// TODO: PAGE_MASK will be moved.
/* HPFAR */
#define PAGE_MASK                   0x00000FFF

#endif
