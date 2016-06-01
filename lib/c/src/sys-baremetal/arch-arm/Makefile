objs-y :=

objs-y += sys_fgetc.o
objs-y += sys_fputc.o
objs-y += sys_morecore.o
objs-y += sys_stdio.o
objs-y += sys_tmpfile.o

obj-y += $(patsubst %, lib/c/src/sys-baremetal/arch-arm/%, ${objs-y})
