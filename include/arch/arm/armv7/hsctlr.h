#ifndef __HSCTLR_H__
#define __HSCTLR_H__

#define HSCTLR_TE        (1 << 30)      /**< Thumb Exception enable. */
#define HSCTLR_EE        (1 << 25)      /**< Exception Endianness. */
#define HSCTLR_FI        (1 << 21)      /**< Fast Interrupts configuration enable. */
#define HSCTLR_WXN       (1 << 19)      /**< Write permission emplies XN. */
#define HSCTLR_I         (1 << 12)      /**< Instruction cache enable.  */
#define HSCTLR_CP15BEN   (1 << 7)       /**< In ARMv7 this bit is RAZ/SBZP. */
#define HSCTLR_C         (1 << 2)       /**< Data or unified cache enable. */
#define HSCTLR_A         (1 << 1)       /**< Alignment check enable. */
#define HSCTLR_M         (1 << 0)       /**< MMU enable. */
#define HSCTLR_BASE      0x30c51878     /**< HSTCLR Base address */


#endif //__HSCTLR_H__
