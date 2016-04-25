#ifndef __HVC_H__
#define __HVC_H__

union hsr {
    uint32_t raw;
    struct {
        uint32_t iss: 25;
        uint32_t il: 1;
        uint32_t ec: 6;
    } entry;
};
typedef union hsr hsr_t;

union iss {
    uint32_t raw: 25;
    struct {
        uint32_t dfsc: 6;
        uint32_t wnr: 1;
        uint32_t s1ptw: 1;
        uint32_t cm: 1;
        uint32_t ea: 1;
        uint32_t reserved: 6;
        uint32_t srt: 4;
        uint32_t zero: 1;
        uint32_t sse: 1;
        uint32_t sas: 2;
        uint32_t isv: 1;
        uint32_t unused: 7;
    } dabt;

    struct {
        uint32_t dir: 1;
        uint32_t CRm: 4;
        uint32_t Rt: 4;
        uint32_t reserved: 1;
        uint32_t CRn: 4;
        uint32_t Opc1: 3;
        uint32_t Opc2: 3;
        uint32_t cond: 4;
        uint32_t cv: 1;
    } cp32;

    struct {
        uint32_t dir: 1;
        uint32_t CRm: 4;
        uint32_t Rt: 4;
        uint32_t reserved: 1;
        uint32_t Rt2: 4;
        uint32_t reserved2: 2;
        uint32_t Opc1: 4;
        uint32_t cond: 4;
        uint32_t cv: 1;
    } cp64;
};
typedef union iss iss_t;

/* HSR Exception Class. */
#define HSR_EC_UNKNOWN        		0x00
#define HSR_EC_WFI_WFE        		0x01
#define HSR_EC_MCR_MRC_CP15   		0x03
#define HSR_EC_MCRR_MRRC_CP15 		0x04
#define HSR_EC_MCR_MRC_CP14   		0x05
#define HSR_EC_LDC_STC_CP14   		0x06
#define HSR_EC_HCRTR_CP0_CP13 		0x07
#define HSR_EC_MRC_VMRS_CP10  		0x08
#define HSR_EC_BXJ    				0x0A
#define HSR_EC_MRRC_CP14  			0x0C
#define HSR_EC_SVC    				0x11
#define HSR_EC_HVC    				0x12
#define HSR_EC_SMC    				0x13
#define HSR_EC_PABT_FROM_GUEST 		0x20
#define HSR_EC_PABT_FROM_HYP_MODE   0x21
#define HSR_EC_DABT_FROM_GUEST 		0x24
#define HSR_EC_DABT_FROM_HYP_MODE   0x25

/* HPFAR */
#define PAGE_MASK                	0x00000FFF

#endif
