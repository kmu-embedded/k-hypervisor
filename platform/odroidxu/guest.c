#include <vm_map.h>
#include <size.h>
#include "devicetree.h"

struct memdesc_t *vm_mmap[NUM_GUESTS_STATIC];


void setup_vm_mmap(void)
{
    vm_mmap[0] = vm_device_md;
}
