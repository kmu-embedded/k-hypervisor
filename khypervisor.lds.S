OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(__start)

#include <arch/arm/rtsm-config.h>

SECTIONS
{
. = CFG_MEMMAP_PHYS_START;
 .text : {
    *(.text);
 }

 .= ALIGN(4);
 .rodata : {
    *(.rodata)
 }

 .= ALIGN(4);
 .data : {
    *(.data)
 }

 .= ALIGN(4);
 PROVIDE(__begin_bss = .);
 .bss : {
    *(.bss)
 }
 PROVIDE(__end_bss = .);

 .stack : {
     __start_stack__ = .;
     . += 0x00100000;   
     . = ALIGN(4);
     __end_stack = .;
 }
}
