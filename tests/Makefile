include ../config.mk

ROOT_PATH	= ..

SRCS		= $(wildcard *.c)
OBJS		= $(SRCS:%.c=%.o)

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	#rm -rf $(OBJS_DIR)

.PHONY: clean
