#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

//#define CONFIG_ENABLE_DEBUG

#if defined (CONFIG_ENABLE_DEBUG)
#define debug_print(fmt, args...)    printf(fmt, #args)
#else
#define debug_print(fmt, args...)
#endif

#endif //__DEBUG_PRINTF_H__
