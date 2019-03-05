#include<io.h>
#include<stdio.h>
#include<drivers/dma/sun4i-dma.h>
#include<size.h>
#include<core/scheduler.h>
struct Queue trans_queue;
void InitQueue(Queue *q) 
{
    q->front = q->rear = NULL;
    q->count = 0;
}

int IsEmpty(Queue *q) 
{
    return q->count == 0;
}

void Enqueue(Queue *q, uint32_t src, uint32_t dst, int bc) 
{
    dma_wait *in = (dma_wait *)malloc(sizeof(dma_wait));
    in->src_addr = src;
    in->dst_addr = dst;
    in->bc = bc; 
    if(IsEmpty(q)){
        q->front = in; 
    }   
    else
    {   
        q->rear->next = in;
    }
    q->rear = in;
    q->count++;
}

dma_wait Dequeue(Queue *q)
{
    dma_wait *zero = (dma_wait*)malloc(sizeof(dma_wait));
    dma_wait *in;

    zero->src_addr = 0;
    zero->dst_addr = 0;
    zero->bc = 0;
    if(IsEmpty(q)){
        printf("fail to Dequeue : queue is empty\n");
        return *zero;
    }
    in = q->front;
    zero->src_addr = in->src_addr;
    zero->dst_addr = in->dst_addr;
    zero->bc = in->bc;
    q->front = in->next;
    free(in);
    q->count--;
    return *zero;
}
void chain_enqueue(uint32_t src_addr, uint32_t dst_addr, int total_bytes){
    // to enqueue when the byte_counters of src_memory to dst_memory is bigger than
    // SZ_128K(because both parameters are 4Bytes, so use SZ_128K)
    int t_weight = 0;
    int bc = SZ_4K;
    int t_bc = total_bytes;
    while(t_bc >= bc){
        Enqueue(&trans_queue, src_addr + t_weight, dst_addr + t_weight, bc);
        t_bc -= bc;
        t_weight += bc;
    }
    if(t_bc < bc && t_bc > 0){
        Enqueue(&trans_queue, src_addr + t_bc, dst_addr + t_bc, t_bc);
    }
}

static irqreturn_t dma_irq_handler(int irq, void *pregs, void *pdata)
{
    // Check channel that can be used by DMAC
    // if pending is not initiated, DMAC do IRQ and doesn't work.
    uint32_t pend_reg_address = DMA_BASE_ADDRESS + SUN4I_DMA_IRQ_PEND_STA_REG;
    uint32_t pend_reg_val = readl(pend_reg_address);
    writel(0, DMA_BASE_ADDRESS + SUN4I_DMA_IRQ_EN_REG);
    writel(pend_reg_val, pend_reg_address);
    
    if(!IsEmpty(&trans_queue)){
        dma_wait *value = (dma_wait*)malloc(sizeof(dma_wait));
        *value = Dequeue(&trans_queue);
        dma_transfer(0, (uint32_t)value->src_addr, (uint32_t)value->dst_addr, value->bc);
    }else{
        printf("put vcpu back\n");
        //register to scheduler again, when every process is finished
        sched_vcpu_register(0,0);
        sched_vcpu_attach(0,0);
    }
    
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
        1. DMA_IRQ_ENABLE_REGISTER
            half * Ns(each even bits) : ON,
            end * Ns(each odd bits) : OFF 
            >> Ns : if you want use p_channel 0, N is 0
        2. NDMA_AUTO_GATING_REGISTER -> ON the bit, 16s
            set as 1 when NDMA is works in Continous mode
     */
    uint32_t irq_enable_reg = 0;
    uint32_t auto_gating_reg = 0;

    irq_enable_reg = (DMA_IRQ_ENABLE_MASK << NDMA0_END_IRQ_ENABLE_SHIFT);
    
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
            >> maximum byte count : 128KB
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
        NDMA_CFG_SET(DEST_BST_LEN, 0x2) | NDMA_CFG_SET(DEST_SEC, 0x1) |
        NDMA_CFG_SET(DEST_ADDR_TYPE, 0x0) | NDMA_CFG_SET(DEST_DRQ_TYPE, SDRAM) |
        NDMA_CFG_SET(BC_MODE_SEL, 0x1) | NDMA_CFG_SET(SRC_DATA_WIDTH, 0x2) |
        NDMA_CFG_SET(SRC_BST_LEN, 0x0) | NDMA_CFG_SET(SRC_SEC, 0x1) |
        NDMA_CFG_SET(SRC_ADDR_TYPE, 0x0) | NDMA_CFG_SET(SRC_DRQ_TYPE, SDRAM);

    writel(cfg_reg, ndma_base_addr + SUN4I_DMA_CFG_REG);
}
