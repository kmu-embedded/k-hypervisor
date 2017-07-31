#include <vm_map.h>
#include <size.h>
#include "devicetree.h"

struct memdesc_t *vm_mmap[CONFIG_NR_VMS];

void setup_vm_mmap(void)
{
    int i;

    for(i = 0; i < CONFIG_NR_VMS; i++) {
        vm_mmap[i] = vm_device_md;
    }
}
