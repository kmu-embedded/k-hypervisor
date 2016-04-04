#ifndef __SERIAL_H__
#define __SERIAL_H__

#ifdef SERIAL_PL01X
#include <drivers/serial_pl01x.h>
#elif SERIAL_SH
#include <drivers/serial_sh.h>
#endif

#endif //__SERIAL_H__
