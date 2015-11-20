CC=gcc

INCLUDES=-I ../../include -I .
LDFLAGS=-lpthread
CFLAGS=-c -Wall $(INCLUDES)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g -ggdb -DDEBUG
else
	CFLAGS += -DNDEBUG
endif

BUILD_DIR = ./
EXECUTABLE = $(BUILD_DIR)/sched_demo.bin

SOURCES = main.c scheduler.c rr.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean builddir

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): builddir $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

builddir:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

