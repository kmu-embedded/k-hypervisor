#ifndef __DEBUG_PRINT_H__
#define __DEBUG_PRINT_H__

#include <stdio.h>

//#define CONFIG_DEBUG_PRINK

#if defined (CONFIG_DEBUG_PRINTK)
#define debug_print(fmt, args...)    printf(fmt, #args)
#else
#define debug_print(fmt, args...)
#endif


#endif //__DEBUG_PRINTF_H__
