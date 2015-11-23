#include <stdio.h>
#include <vcpu.h>

#define VCPU_TOTAL_NUM    10
#define VCPU_NUM          5

void TEST_VCPU();

struct vcpu *vcpu[VCPU_TOTAL_NUM];

void TEST_VCPU()
{
    int i = 0;

    printf("============%s=================\n", __func__);
    for(i = 0; i < VCPU_NUM; i++) {
        if((vcpu[i] = vcpu_create()) == VCPU_CREATE_FAILED){
            printf("%dth vcpu creat fail\n", i);
        }

        if(vcpu_init(vcpu[i]) != VCPU_REGISTERED){
            printf("%dth vcpu init fail\n", i);
        }

        if(vcpu_start(vcpu[i]) != VCPU_ACTIVATED){
            printf("%dth vcpu start fail\n", i);
        }
    }
    print_all_vcpu();
    printf("====%d vcpu delete\n", 0);
    if(vcpu_delete(vcpu[0]) != VCPU_UNDEFINED){
        printf("ID : 0 vcpu delete failed\n");
    }
    print_all_vcpu();
    printf("====%d vcpu delete\n", 3);
    if(vcpu_delete(vcpu[3]) !=  VCPU_UNDEFINED){
        printf("ID : 3 vcpu delete failed\n");
    }
    print_all_vcpu();
    printf("====%d vcpu delete\n", 2);
    if(vcpu_delete(vcpu[2]) != VCPU_UNDEFINED){
        printf("ID : 2 vcpu delete failed\n");
    }
    print_all_vcpu();
    printf("====%d vcpu delete\n", 1);
    if(vcpu_delete(vcpu[1]) != VCPU_UNDEFINED){
        printf("ID : 1 vcpu delete failed\n");
    }
    print_all_vcpu();
    printf("====%d vcpu delete\n", 4);
    if(vcpu_delete(vcpu[4]) != VCPU_UNDEFINED){
        printf("ID : 4 vcpu delete failed\n");
    }
    print_all_vcpu();
}

