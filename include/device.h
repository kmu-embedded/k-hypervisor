#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>

struct device {

    int32_t (* init) ();

};



#endif /* __DEVICE_H__ */
