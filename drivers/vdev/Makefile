include  ../../config.mk

ROOT_PATH	= ../..
#OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/core/vdev

INCLUDES += -I./

SRCS		= $(wildcard *.c)
OBJS		= $(SRCS:%.c=%.o)

all: $(OBJS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@
