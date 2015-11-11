#include <stdio.h>
#include <vm.h>

#define NUM_OF_VMCB 2

int main()
{
    int i = 0;
    unsigned int vms[NUM_OF_VMCB] = {-1,};

    for(i = 0 ; i < NUM_OF_VMCB ; i++){
        vms[i] = vm_create(1);
        printf("vmid : %d\n", vms[i]);
    }

    return 0;
}
