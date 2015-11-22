/*
 * This main function is just for experiment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheduler.h>

int main() {
    int cnt;
    printf("Hello World!\n");

    sched_init();

    sched_vcpu_register(0);
    sched_vcpu_register(1);
    sched_vcpu_register(2);
    sched_vcpu_register(3);

    sched_vcpu_attach(1);
    sched_vcpu_attach(2);
    sched_vcpu_attach(0);
    sched_vcpu_attach(3);

    cnt = 100;
    do {
        do_schedule();
    } while (--cnt);

    // sched_vcpu_detach();
    // sched_vcpu_unregister();

    printf("Bye World!\n");

    return 0;
}
