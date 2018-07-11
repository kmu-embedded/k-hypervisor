#include<io.h>
#include<stdio.h>
#include<drivers/dma/sun4i-dma.h>

static irqreturn_t dma_irq_handler(int irq, void *pregs, void *pdata)
{
    printf("DMA IRQ[%d] is registered\n", irq);
    writel(0, DMA_BASE_ADDRESS + SUN4I_DMA_IRQ_EN_REG);

    return VMM_IRQ; 
}

void dma_irq_enable()
{
    // TODO: IRQ_LEVEL_SENSITIVE seems weird!
    register_irq_handler(59, &dma_irq_handler, /*IRQ_LEVEL_SENSITIVE*/ 0);
}

void dma_reg_init()
{
    /*
        to use dma, need to set base registers
        1. DMA_IRQ_ENABLE_REGISTER -> half * Ns(each even bits)
            : ON, end * Ns(each odd bits) : OFF 
            >> Ns : if you want use p_channel 0, N is 0
        2. NDMA_AUTO_GATING_REGISTER -> ON the bit, 16s
     */
    uint32_t irq_enable_reg = 0;
    uint32_t auto_gating_reg = 0;

    irq_enable_reg = (DMA_IRQ_ENABLE_MASK << NDMA0_HALF_IRQ_ENABLE_SHIFT)
        & (~(DMA_IRQ_ENABLE_MASK) << NDMA0_END_IRQ_ENABLE_SHIFT);
    auto_gating_reg = NDMA_AUTO_GATING_REG_MASK;

    writel(irq_enable_reg, DMA_BASE_ADDRESS + SUN4I_DMA_IRQ_EN_REG);
    writel(auto_gating_reg, DMA_BASE_ADDRESS + SUN4I_NDMA_AUTO_GATING_REG);

}

void dma_transfer(int pchan, uint32_t src_addr, uint32_t dst_addr, int bc)
{
   /*
        src = address of src data
        dst = address of dst user want 
        bc(length of src data memory) = byte count
    */
    uint32_t ndma_base_addr = DMA_BASE_ADDRESS + NDMA_REG_BASE_ADDRESS(pchan);

    /* dma reg init */
    dma_reg_init();
    
    /* configure pchannel */
    writel((uint32_t)src_addr, ndma_base_addr + SUN4I_DMA_SRC_ADDR_REG);
    writel((uint32_t)dst_addr, ndma_base_addr + SUN4I_DMA_DEST_ADDR_REG);
    writel((uint32_t)bc, ndma_base_addr + SUN4I_DMA_BYTE_COUNT_REG);

    ndma_configure(ndma_base_addr);
}

void ndma_configure(uint32_t ndma_base_addr)
{
    /*
       init registers to use dma.
     */
    uint32_t cfg_reg = 0;

    /* set the bit */
    cfg_reg = NDMA_CFG_SET(LOADING,0x1) | NDMA_CFG_SET(CONTI_MODE_EN,0x1) |
        NDMA_CFG_SET(WAIT_STATE, 0x7) | NDMA_CFG_SET(DEST_DATA_WIDTH, 0x2) |
        NDMA_CFG_SET(DEST_BST_LEN, 0x0) | NDMA_CFG_SET(DEST_SEC, 0x1) |
        NDMA_CFG_SET(DEST_ADDR_TYPE, 0x0) | NDMA_CFG_SET(DEST_DRQ_TYPE, SDRAM) |
        NDMA_CFG_SET(BC_MODE_SEL, 0x1) | NDMA_CFG_SET(SRC_DATA_WIDTH, 0x2) |
        NDMA_CFG_SET(SRC_BST_LEN, 0x0) | NDMA_CFG_SET(SRC_SEC, 0x1) |
        NDMA_CFG_SET(SRC_ADDR_TYPE, 0x0) | NDMA_CFG_SET(SRC_DRQ_TYPE, SDRAM);

    writel(cfg_reg, ndma_base_addr + SUN4I_DMA_CFG_REG);
}
