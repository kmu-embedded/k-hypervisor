#include <memory.h>
#include <guest_memory_hw.h>

hvmm_status_t memory_init(struct memmap_desc **guest0, struct memmap_desc **guest1)
{
    return memory_hw_init(guest0, guest1);
}

hvmm_status_t memory_save(void)
{
    return memory_hw_save();
}

hvmm_status_t memory_restore(vmid_t vmid)
{
    return memory_hw_restore(vmid);
}

