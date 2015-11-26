#ifndef __PRINT_H__
#define __PRINT_H__

#include <drivers/pl01x.h>

void init_print();

void printf(const char *format, ...);

#endif
