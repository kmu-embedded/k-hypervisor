objs-y :=

objs-y += head.o
objs-y += context.o
objs-y += mutex.o
objs-y += vector.o
objs-y += smccc.o

obj-y += $(patsubst %, arch/arm/v7/%, ${objs-y})
