objs-y :=

objs-$(CONFIG_PL01X_SERIAL) += serial_pl01x.o
objs-$(CONFIG_SH_SERIAL) += serial_sh.o
objs-$(CONFIG_S5P_SERIAL) += serial_s5p.o

obj-y += $(patsubst %, drivers/serial/%, ${objs-y})
