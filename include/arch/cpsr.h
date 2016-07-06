#ifndef __CPSR_H__
#define __CPSR_H__

#define CPSR_MODE_USER  		0x10
#define CPSR_MODE_FIQ   		0x11
#define CPSR_MODE_IRQ   		0x12
#define CPSR_MODE_SVC   		0x13
#define CPSR_MODE_MON   		0x16
#define CPSR_MODE_ABT   		0x17
#define CPSR_MODE_HYP   		0x1A
#define CPSR_MODE_UND   		0x1B
#define CPSR_MODE_SYS   		0x1F

#define CPSR_MODE_MASK           0X1F
#define CPSR_ASYNC_ABT_DIABLE    0x100
#define CPSR_IRQ_DISABLE         0x80
#define CPSR_FIQ_DISABLE         0x40

#define CPSR_ASYNC_ABT_BIT  	CPSR_ASYNC_ABT_DIABLE
#define CPSR_IRQ_BIT        	CPSR_IRQ_DISABLE
#define CPSR_FIQ_BIT        	CPSR_FIQ_DISABLE


#endif /* __CPSR_H__ */
