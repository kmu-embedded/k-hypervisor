#ifndef __HVC_H__
#define __HVC_H__

#include <stdint.h>
#include <arch/armv7.h>

struct iss_abt {
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
};

struct iss_cp32{
    uint32_t dir: 1;
    uint32_t CRm: 4;
    uint32_t Rt: 4;
    uint32_t reserved: 1;
    uint32_t CRn: 4;
    uint32_t Opc1: 3;
    uint32_t Opc2: 3;
    uint32_t cond: 4;
    uint32_t cv: 1;
};

struct iss_cp64 {
    uint32_t dir: 1;
    uint32_t CRm: 4;
    uint32_t Rt: 4;
    uint32_t reserved: 1;
    uint32_t Rt2: 4;
    uint32_t reserved2: 2;
    uint32_t Opc1: 4;
    uint32_t cond: 4;
    uint32_t cv: 1;
};

/* HSR Exception Class. */
#define UNK         		0x00
#define WFI_WFE        		0x01
#define MCR_MRC_CP15   		0x03
#define MCRR_MRRC_CP15 		0x04
#define MCR_MRC_CP14   		0x05
#define LDC_STC_CP14   		0x06
#define HCRTR_CP0_CP13 		0x07
#define MRC_VMRS_CP10  		0x08
#define BXJ    				0x0A
#define MRRC_CP14  			0x0C
#define SVC    				0x11
#define HVC    				0x12
#define SMC    				0x13
#define PABT_FROM_GUEST 	0x20
#define PABT_FROM_HYP       0x21
#define DABT_FROM_GUEST 	0x24
#define DABT_FROM_HYP       0x25

/* HPFAR */
#define PAGE_MASK                	0x00000FFF

int handle_data_abort(struct core_regs *regs, uint32_t iss);
#endif
