#ifndef __SUN4I_DMA_H__
#define __SUN4I_DMA_H__
#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/irq.h>
#include <lib/list.h>
#include <stdlib.h>

//TODO: For now, we only defined NDMA related things.
//      Define everything related DMA later.

/* device DMA base address */
#define DMA_BASE_ADDRESS            0x01C02000
/* device DMA register offset */

#define SUN4I_DMA_IRQ_EN_REG        0x0
#define SUN4I_DMA_IRQ_PEND_STA_REG  0x4
#define SUN4I_NDMA_AUTO_GATING_REG  0x8 //

#define NDMA_REG_BASE_ADDRESS(PCHAN) (0x100 + (PCHAN * 0x20))

/* NDMA register offset */
#define SUN4I_DMA_CFG_REG           0x0
#define SUN4I_DMA_SRC_ADDR_REG      0x4
#define SUN4I_DMA_DEST_ADDR_REG     0x8
#define SUN4I_DMA_BYTE_COUNT_REG    0xC

#define DDMA_REG_BASE_ADDRESS(PCHAN) (0x300 + (PCHAN * 0x20))

/* irq enable register setting */
#define DMA_IRQ_ENABLE_MASK         0x1

#define NDMA0_HALF_IRQ_ENABLE_SHIFT     0
#define NDMA0_END_IRQ_ENABLE_SHIFT      1
#define NDMA1_HALF_IRQ_ENABLE_SHIFT     2
#define NDMA1_END_IRQ_ENABLE_SHIFT      3
#define NDMA2_HALF_IRQ_ENABLE_SHIFT     4
#define NDMA2_END_IRQ_ENABLE_SHIFT      5
#define NDMA3_HALF_IRQ_ENABLE_SHIFT     6
#define NDMA3_END_IRQ_ENABLE_SHIFT      7
#define NDMA4_HALF_IRQ_ENABLE_SHIFT     8
#define NDMA4_END_IRQ_ENABLE_SHIFT      9
#define NDMA5_HALF_IRQ_ENABLE_SHIFT     10
#define NDMA5_END_IRQ_ENABLE_SHIFT      11
#define NDMA6_HALF_IRQ_ENABLE_SHIFT     12
#define NDMA6_END_IRQ_ENABLE_SHIFT      13
#define NDMA7_HALF_IRQ_ENABLE_SHIFT     14
#define NDMA7_END_IRQ_ENABLE_SHIFT      15

#define NDMA_LOADING_SHIFT          31 
#define NDMA_CONTI_MODE_EN_SHIFT    30
#define NDMA_WAIT_STATE_SHIFT       27
#define NDMA_DEST_DATA_WIDTH_SHIFT  25
#define NDMA_DEST_BST_LEN_SHIFT     23
#define NDMA_DEST_SEC_SHIFT         22
#define NDMA_DEST_ADDR_TYPE_SHIFT   21
#define NDMA_DEST_DRQ_TYPE_SHIFT    16 
#define NDMA_BC_MODE_SEL_SHIFT      15
// 14:11 -> NONE
#define NDMA_SRC_DATA_WIDTH_SHIFT   9
#define NDMA_SRC_BST_LEN_SHIFT      7
#define NDMA_SRC_SEC_SHIFT          6
#define NDMA_SRC_ADDR_TYPE_SHIFT    5
#define NDMA_SRC_DRQ_TYPE_SHIFT     0

#define NDMA_LOADING_MASK           0x1
#define NDMA_CONTI_MODE_EN_MASK     0x1 
#define NDMA_WAIT_STATE_MASK        0x7
#define NDMA_DEST_DATA_WIDTH_MASK   0x3
#define NDMA_DEST_BST_LEN_MASK      0x3 
#define NDMA_DEST_SEC_MASK          0x1 
#define NDMA_DEST_ADDR_TYPE_MASK    0x1 
#define NDMA_DEST_DRQ_TYPE_MASK     0x1F
#define NDMA_BC_MODE_SEL_MASK       0x1 
#define NDMA_SRC_DATA_WIDTH_MASK    0x3
#define NDMA_SRC_BST_LEN_MASK       0x3 
#define NDMA_SRC_SEC_MASK           0x1 
#define NDMA_SRC_ADDR_TYPE_MASK     0x1 
#define NDMA_SRC_DRQ_TYPE_MASK      0x1F

#define SDRAM 0x16

#define NDMA_CFG_SET(field, value) \
    (((value) & NDMA_##field##_MASK) << NDMA_##field##_SHIFT)

/* set when especially( read the ARM v7-AR reference ) */
#define NDMA_AUTO_GATING_REG_MASK 0x1 << 16 

typedef struct dma_wait
{
    uint32_t src_addr;
    uint32_t dst_addr;
    int bc;
    struct dma_wait *next;
}dma_wait;

typedef struct Queue
{
    dma_wait *front;
    dma_wait *rear;
    int count;
}Queue;

void InitQueue(Queue *q);
int IsEmpty(Queue *q);
void Enqueue(Queue *q, uint32_t src, uint32_t dst, int bc);
dma_wait Dequeue(Queue *q);
void chain_enqueue(uint32_t src_addr, uint32_t dst_addr, int total_bytes);

void dma_irq_enable();
void dma_reg_init();
void dma_transfer(int pchan, uint32_t src_addr, uint32_t dst_addr, int bc);
void ndma_configure(uint32_t ndma_base_addr);


#endif
