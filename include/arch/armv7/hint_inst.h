#ifndef __HINT_INSTRUCTION_H__
#define __HINT_INSTRUCTION_H__

#include <asm/asm.h>

#define sev()   	asm __volatile__ ("sev" : : : "memory")
#define wfe()   	asm __volatile__ ("wfe" : : : "memory")
#define wfi()   	asm __volatile__ ("wfi" : : : "memory")
#define nop()		asm __volatile__ ("nop")
#define yield()		asm __volatile__ ("yield")

#endif /* __HINT_INSTRUCTION_H__ */
