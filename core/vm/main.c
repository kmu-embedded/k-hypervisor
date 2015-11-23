extern TEST_VMCB();
extern TEST_VCPU();

void hypervisor_init();

int main()
{
    hypervisor_init();

    TEST_VMCB();
    TEST_VCPU();
    return 0;
}
void hypervisor_init()
{
    vm_setup();
}
