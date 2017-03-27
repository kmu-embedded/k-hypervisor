#ifndef __CPSR_H__
#define __CPSR_H__

/* Define CPSR(SPSR) and processor modes at B1.3 section in DDI0406C_C */

#define CPSR_N                  31
#define CPSR_Z                  30
#define CPSR_C                  29
#define CPSR_V                  28
#define CPSR_Q                  27
#define CPSR_IT0                25
#define CPSR_J                  24
#define CPSR_RESERVED           20
#define CPSR_GE                 16
#define CPSR_IT1                10
#define CPSR_E                  9
#define CPSR_A                  8
#define CPSR_I                  7
#define CPSR_F                  6
#define CPSR_T                  5
#define CPSR_M                  0

#define N                       (0b1)
#define Z                       (0b1)
#define C                       (0b1)
#define V                       (0b1)
#define Q                       (0b1)
#define IT0                     (0b11)
#define J                       (0b11)
// #define RESERVED                (0b1111)
#define GE                      (0b1111)
#define IT1                     (0b111111)
#define E                       (0b1)
#define A                       (0b1)
#define I                       (0b1)
#define F                       (0b1)
#define T                       (0b1)
#define M                       (0b11111)

#define MODE_USR                (0b10000)
#define MODE_FIQ                (0b10001)
#define MODE_IRQ                (0b10010)
#define MODE_SVC                (0b10011)
#define MODE_MON                (0b10110)
#define MODE_ABT                (0b10011)
#define MODE_HYP                (0b11010)
#define MODE_UND                (0b11011)
#define MODE_SYS                (0b11111)

#define CPSR_BIT(x)             ( x << CPSR_##x )
#define CPSR_MODE(x)            ( MODE_##x << CPSR_M )
#define CPSR(x)                 ( x )

#endif /* __CPSR_H__ */
