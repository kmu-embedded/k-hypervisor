#ifndef __SERIAL_H__
#define __SERIAL_H__

#ifdef CONFIG_PL01X_SERIAL
#include <drivers/serial_pl01x.h>
#elif CONFIG_SH_SERIAL
#include <drivers/serial_sh.h>
#elif CONFIG_S5P_SERIAL
#include <drivers/serial_s5p.h>
#endif

#endif //__SERIAL_H__
