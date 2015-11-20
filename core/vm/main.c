#include <stdio.h>
#include <vm.h>

#define VM_TOTAL_NUM      10
#define VM_NUM            5
#define VM_VCPU_NUM       1

unsigned char vmcb[VM_TOTAL_NUM] = {0, };

void TEST_VMCB();
void hypervisor_init();

int main()
{
    TEST_VMCB();
    return 0;
}

void TEST_VMCB()
{
    int i = 0;

    hypervisor_init();

    for(i = 0; i < VM_NUM; i++) {
        if((vmcb[i] = vm_create(VM_VCPU_NUM)) == VM_CREATE_FAILED){
            printf("%dth vm creat fail\n", i);
        }
        if(vm_init(vmcb[i]) != HALTED){
            printf("%dth vm init fail\n", i);
        }
        if(vm_start(vmcb[i]) != RUNNING){
            printf("%dth vm start fail\n", i);
        }

    }
    print_all_vm();

    printf("====%d vm delete\n", 5);
    if(vm_delete(5) != UNDEFINED){
        printf("ID : 5 vm delete failed\n");
    }
    print_all_vm();

    printf("====%d vm delete\n", vmcb[0]);
    if(vm_delete(vmcb[0]) != UNDEFINED){
        printf("ID : 0 vm delete failed\n");
    }
    print_all_vm();
    printf("====%d vm delete\n", vmcb[3]);
    if(vm_delete(vmcb[3]) !=  UNDEFINED){
        printf("ID : 3 vm delete failed\n");
    }
    print_all_vm();
    printf("====%d vm delete\n", vmcb[2]);
    if(vm_delete(vmcb[2]) != UNDEFINED){
        printf("ID : 2 vm delete failed\n");
    }
    print_all_vm();
    printf("====%d vm delete\n", vmcb[1]);
    if(vm_delete(vmcb[1]) != UNDEFINED){
        printf("ID : 1 vm delete failed\n");
    }
    print_all_vm();
    printf("====%d vm delete\n", vmcb[4]);
    if(vm_delete(vmcb[4]) != UNDEFINED){
        printf("ID : 4 vm delete failed\n");
    }
    print_all_vm();
}

void hypervisor_init()
{
     vm_setup();
}
