#include <stdio.h>
#include <vm.h>

#define NUM_OF_VM 10

#define VM_NUM      10
#define VCPU_NUM    1

int main()
{
    int i = 0;
    int vmid;

    for(i = 0; i < VM_NUM; i++) {
        if((vmid = vm_create(VCPU_NUM)) < HVMM_STATE_SUCCESS)
        {
            printf("VM CREATE FIALED\n");
            continue;
        }
        printf("i : %d , vmid : %d\n", i, vmid);
    }
    return 0;
}
