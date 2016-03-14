#include <rtsm-config.h>

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(__start)

INPUT(../guestimages/guest0.bin)
INPUT(../guestimages/guest1.bin)
INPUT(../guestimages/guest2.bin)

HEAP_SIZE   = 0x04000000;

SECTIONS
{
    . = CFG_MEMMAP_PHYS_START;
    /* Guest 0 */
    . = CFG_MEMMAP_GUEST0_ATAGS_OFFSET;
    . = CFG_MEMMAP_GUEST0_OFFSET;
    _guest0_bin_start = .;
    .guest0 : { ../guestimages/guest0.bin}
    _guest0_bin_end = .;

    . = ALIGN(4);

    /* Guest 1*/
    . = CFG_MEMMAP_GUEST1_ATAGS_OFFSET;
    . = CFG_MEMMAP_GUEST1_OFFSET;
    _guest1_bin_start = .;
    .guest1 : { ../guestimages/guest1.bin }
    _guest1_bin_end = .;

    . = ALIGN(4);

    /* Guest 2 */
    . = CFG_MEMMAP_GUEST2_ATAGS_OFFSET;
    . = CFG_MEMMAP_GUEST2_OFFSET;
    _guest2_bin_start = .;
    .guest2 : { ../guestimages/guest2.bin }
    _guest2_bin_end = .;

    . = CFG_HYP_START_ADDRESS;
    . = ALIGN(4);
    .text :
    {
        *(.text.vector)
        *(.text.startup)
        *(.text)
        __vdev_module_high_start = .;
        *(.vdev_module0.init)
        __vdev_module_high_end = .;
        *(.vdev_module1.init)
        __vdev_module_middle_end = .;
        *(.vdev_module2.init)
        __vdev_module_low_end = .;
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
        . = . + 0x805000 * NUM_GUESTS_STATIC;
        . = ALIGN(4);
    }
    PROVIDE(__END_VM_PGTABLE = .);
}
