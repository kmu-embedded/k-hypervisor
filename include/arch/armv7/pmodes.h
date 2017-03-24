#ifndef __PROCESSOR_MODES_H__
#define __PROCESSOR_MODES_H__

/* Define the processor modes at B1.3 section in DDI0406C_C */

#define USR_MODE        (0b10000)
#define FIQ_MODE        (0b10001)
#define IRQ_MODE        (0b10010)
#define SVC_MODE        (0b10011)
#define MON_MODE        (0b10110)
#define ABT_MODE        (0b10011)
#define HYP_MODE        (0b11010)
#define UND_MODE        (0b11011)
#define SYS_MODE        (0b11111)

#endif
