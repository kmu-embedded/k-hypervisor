#include <vmem.h>

void vmem_setup()
{
    memory_hw_setup();
}

void vmem_create()
{
    // TODO(casionwoo) : This will be implemented after making struct vmem.
}

hvmm_status_t vmem_init(struct vmem *vmem, vmid_t vmid)
{
    return memory_hw_init(vmem->memmap, &vmem->pgtable_base, vmid);
}

hvmm_status_t vmem_save(void)
{
    return memory_hw_save();
}

hvmm_status_t vmem_restore(struct vmem *vmem, vmid_t vmid)
{
    return memory_hw_restore(vmid, &vmem->pgtable_base);
}

