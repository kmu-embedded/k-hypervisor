objs-y :=

SUBDIRECTORIES += platform/${TARGET}

objs-y += interrupt.o
objs-y += guest.o
objs-y += platform.o

obj-y += $(patsubst %, platform/${TARGET}/%, ${objs-y})
