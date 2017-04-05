#include <config.h>

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(__start)

HEAP_SIZE   = 0x00010000;

SECTIONS
{
    . = CFG_HYP_START_ADDRESS;
    . = ALIGN(4);
    .text :
    {
        *(.text.vector)
        *(.text.startup)
        *(.text)
        __vdev_module_start = .;
        *(.vdev_module.init)
        __vdev_module_end = .;
    }
    _end_text = .;

    . = ALIGN(4);
    .init :
    {
        *(.init.head)
        *(.init)
        *(.init.*)
    }
    _end_init = ALIGN(4);

    . = ALIGN(4);
    .rodata : {
        *(.rodata)
    }

    . = ALIGN(4);
    .data : {
        *(.data)
    }

    . = ALIGN(0x1000);
    PROVIDE(__begin_bss = .);
    .bss : {
        *(.bss)
        . = ALIGN(4);
    }
    PROVIDE(__end_bss = .);

    . = ALIGN(0x1000);
    PROVIDE(__begin_heap = .);
    .heap : {
        *(.heap)
        . = . + HEAP_SIZE;
        . = ALIGN(4);
    }
    PROVIDE(__end_heap = .);

    . = ALIGN(0x1000);

    .stack : {
        __start_stack__ = .;
        . = . + (HYP_STACK_SIZE * NR_CPUS);
        . = ALIGN(4);
        __end_stack = .;
    }

    . = ALIGN(0x1000);
    PROVIDE(__HYP_PGTABLE = .);
    .pgtable : {
        *(.pgtable)
        . = . + 0x805000;
        . = ALIGN(4);
    }
    PROVIDE(__END_HYP_PGTABLE = .);

    . = ALIGN(0x1000);
    PROVIDE(__VM_PGTABLE = .);
    .vm_pgtable : {
        *(.vm_pgtable)
        . = . + 0x805000 * CONFIG_NR_VMS;
        . = ALIGN(4);
    }
    PROVIDE(__END_VM_PGTABLE = .);
    __end = .;
}
