#ifndef __PL180_MCI_H__
#define __PL180_MCI_H__


#define PL180_BASE      	0x1C050000

#define MCI_POWER(x)		(x + 0x000)		// RW
#define MCI_CLOCK(x)		(x + 0x004)		// RW
#define MCI_ARG(x)       	(x + 0x008)		// RW
#define MCI_CMD(x)			(x + 0x00C)		// RW
#define MCI_RES_CMD(x)		(x + 0x010)		// RW

#define MCI_RES0(x)			(x + 0x014)		// RO
#define MCI_RES1(x)			(x + 0x018)		// RO
#define MCI_RES2(x)        	(x + 0x01C)		// RO
#define MCI_RES3(x)			(x + 0x020)		// RO

#define MCI_DTIMER(x)       (x + 0x024)		// RW
#define MCI_DLENGTH(x)		(x + 0x028)		// RW
#define MCI_DCTRL(x)		(x + 0x02C)		// RW
#define MCI_DCNT(x)			(x + 0x030)		// RW

#define MCI_STATUS(x)		(x + 0x034)		// RO
#define MCI_CLEAR(x)		(x + 0x038)		// WO

#define MCI_MASK0(x)		(x + 0x03C)		//RW
#define MCI_MASK1(x)		(x + 0x040)		//RW
#define MCI_SELECT(x)		(x + 0x044)		//RW

#define MCI_FIFO_CNT(x)		(x + 0x048)		//RO
#define MCI_FIFO(x)			(x + 0x080)		//RW

#define MCI_PERIPHID0(x)	(x + 0xFE0)		//RO
#define MCI_PERIPHID1(x)	(x + 0xFE4)		//RO
#define MCI_PERIPHID2(x)	(x + 0xFE8)		//RO
#define MCI_PERIPHID3(x)	(x + 0xFEC)		//RO

#define MCI_PCELL0(x)		(x + 0xFF0)		//RO
#define MCI_PCELL1(x)		(x + 0xFF4)		//RO
#define MCI_PCELL2(x)		(x + 0xFF8)		//RO
#define MCI_PCELL3(x)		(x + 0xFFC)		//RO

#endif
