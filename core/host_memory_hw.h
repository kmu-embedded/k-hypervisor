#ifndef __HOST_MEMORY_H__
#define __HOST_MEMORY_H__

void host_memory_init(void);
int memory_enable(void);

#define INITIAL_MAIR0VAL 0xeeaa4400
#define INITIAL_MAIR1VAL 0xff000004
#define INITIAL_MAIRVAL (INITIAL_MAIR0VAL|INITIAL_MAIR1VAL<<32)

/**
 * \defgroup SCTLR
 *
 * System Control Register.
 * The SCTLR provides the top level control of the system, including its memory
 * system.
 * - HSCTLR is a subset of this
 *
 * @{
 */
#define SCTLR_TE        (1<<30) /**< Thumb Exception enable. */
#define SCTLR_AFE       (1<<29) /**< Access Flag Enable. */
#define SCTLR_TRE       (1<<28) /**< TEX Remp Enable. */
#define SCTLR_NMFI      (1<<27) /**< Non-Maskable FIQ(NMFI) support. */
#define SCTLR_EE        (1<<25) /**< Exception Endianness. */
#define SCTLR_VE        (1<<24) /**< Interrupt Vectors Enable. */
#define SCTLR_U         (1<<22) /**< In ARMv7 this bit is RAO/SBOP. */
#define SCTLR_FI        (1<<21) /**< Fast Interrupts configuration enable. */
#define SCTLR_WXN       (1<<19) /**< Write permission emplies XN. */
#define SCTLR_HA        (1<<17) /**< Hardware Access flag enable. */
#define SCTLR_RR        (1<<14) /**< Round Robin select. */
#define SCTLR_V         (1<<13) /**< Vectors bit. */
#define SCTLR_I         (1<<12) /**< Instruction cache enable.  */
#define SCTLR_Z         (1<<11) /**< Branch prediction enable. */
#define SCTLR_SW        (1<<10) /**< SWP and SWPB enable. */
#define SCTLR_B         (1<<7)  /**< In ARMv7 this bit is RAZ/SBZP. */
#define SCTLR_C         (1<<2)  /**< Cache enable. */
#define SCTLR_A         (1<<1)  /**< Alignment check enable. */
#define SCTLR_M         (1<<0)  /**< MMU enable. */
#define SCTLR_BASE        0x00c50078  /**< STCLR Base address */
#define HSCTLR_BASE       0x30c51878  /**< HSTCLR Base address */
/** @}*/

/**
 * \defgroup HTTBR
 * @brief Hyp Translation Table Base Register
 *
 * The HTTBR holds the base address of the translation table for the stasge-1
 * translation of memory accesses from Hyp mode.
 * @{
 */
#define HTTBR_INITVAL               0x0000000000000000ULL
#define HTTBR_BADDR_MASK            0x000000FFFFFFF000ULL
#define HTTBR_BADDR_SHIFT           12
/** @}*/

/**
 * \defgroup HTCR
 * @brief Hyp Translation Control Register
 *
 * The HTCR controls the translation table walks required for the stage-1
 * translation of memory accesses from Hyp mode, and holds cacheability and
 * shareability information for the accesses.
 * @{
 */
#define HTCR_INITVAL                0x80000000
#define HTCR_SH0_MASK               0x00003000 /**< \ref Shareability */
#define HTCR_SH0_SHIFT              12
#define HTCR_ORGN0_MASK             0x00000C00 /**< \ref Outer_cacheability */
#define HTCR_ORGN0_SHIFT            10
#define HTCR_IRGN0_MASK             0x00000300 /**< \ref Inner_cacheability */
#define HTCR_IRGN0_SHIFT            8
#define HTCR_T0SZ_MASK              0x00000003
#define HTCR_T0SZ_SHIFT             0
/** @} */
#endif //__HOST_MEMORY_H__

