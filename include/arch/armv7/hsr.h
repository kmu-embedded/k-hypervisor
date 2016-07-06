#ifndef __HSR_H__
#define __HSR_H__

#include <arch/armv7.h>

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

#endif
