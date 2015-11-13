#include <stdio.h>
#include <vm.h>

#define NUM_OF_VM 10

int main()
{
    int i = 0;
    int vm[NUM_OF_VM] = {-1,};

    for(i = 0; i < NUM_OF_VM; i++){
        if((vm[i] = vm_create(1)) < HVMM_STATE_SUCCESS)
        {
            printf("VM CREATE FIALED\n");
            continue;
        }
        
        printf("i : %d , vmid : %d\n", i, vm[i]);
    }
    
    return 0;
}
