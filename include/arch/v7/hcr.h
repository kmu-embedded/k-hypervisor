#ifndef __HCR_H__
#define __HCR_H__

#define HCR_TGE                     27
#define HCR_TVE                     26
#define HCR_TTLB                    25
#define HCR_TPU                     24
#define HCR_TPC                     23
#define HCR_TSW                     22
#define HCR_TAC                     21
#define HCR_TIDCP                   20
#define HCR_TSC                     19
#define HCR_TID3                    18
#define HCR_TID2                    17
#define HCR_TID1                    16
#define HCR_TID0                    15
#define HCR_TWE                     14
#define HCR_TWI                     13
#define HCR_DC                      12
#define HCR_BSU                     10
#define HCR_FB                      9
#define HCR_VA                      8
#define HCR_VI                      7
#define HCR_VF                      6
#define HCR_AMO                     5
#define HCR_IMO                     4
#define HCR_FMO                     3
#define HCR_PTW                     2
#define HCR_SWIO                    1
#define HCR_VM                      0


#define TGE_BIT                     (0b1)
#define TVE_BIT                     (0b1)
#define TTLB_BIT                    (0b1)
#define TPU_BIT                     (0b1)
#define TPC_BIT                     (0b1)
#define TSW_BIT                     (0b1)
#define TAC_BIT                     (0b1)
#define TIDCP_BIT                   (0b1)
#define TSC_BIT                     (0b1)
#define TID3_BIT                    (0b1)
#define TID2_BIT                    (0b1)
#define TID1_BIT                    (0b1)
#define TID0_BIT                    (0b1)
#define TWE_BIT                     (0b1)
#define TWI_BIT                     (0b1)
#define DC_BIT                      (0b1)
#define BSU_BIT                     (0b11)
#define FB_BIT                      (0b1)
#define VA_BIT                      (0b1)
#define VI_BIT                      (0b1)
#define VF_BIT                      (0b1)
#define AMO_BIT                     (0b1)
#define IMO_BIT                     (0b1)
#define FMO_BIT                     (0b1)
#define PTW_BIT                     (0b1)
#define SWIO_BIT                    (0b1)
#define VM_BIT                      (0b1)

#define HCR_BIT(x)                  ( x##_BIT << HCR_##x )

#endif //__HCR_H__
