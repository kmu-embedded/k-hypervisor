OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)
ENTRY(__start)

#include <rtsm-config.h>

SECTIONS
{
 . = CFG_MEMMAP_PHYS_START;
 . = CFG_MEMMAP_PHYS_START + 0x8000 - 0x40;
 . = CFG_MEMMAP_PHYS_START + 0x00d00000;

 . = CFG_MEMMAP_MON_OFFSET;

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
 begin_bss = .;
 .bss : {
    *(.bss)
 }
 end_bss = .;

 .stack : {
     __start_stack__ = .;
     . += 0x00100000;   
     . = ALIGN(4);
     __end_stack = .;
 }
}
