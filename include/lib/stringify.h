#ifndef __STRINGIFY_H__
#define __STRINGIFY_H__

#define __stringify_1(x...) #x
#define __stringify(x...)   __stringify_1(x)

#endif /* stringify.h */
