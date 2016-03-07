# Author has to know the difference among '?=', '=', '+=' and ':='

CROSS_COMPILE	= arm-linux-gnueabihf-
PROCESSOR		= cortex-a15

DEBUG_FLAG		= -g
CONFIG_FLAG		= -mcpu=$(PROCESSOR) -marm

MACHINE			= rtsm
PROJECT			= khypervisor
TARGET			= khypervisor-$(MACHINE)

# Desciption of directory structure
ROOT			= $(PWD)
ARCH			= arch/arm
VERSION			= v7
CORE			= core
TESTS			= tests
LIB				= lib
DRV				= drivers
INC				= include
MKDIR			= mkdir
BUILD			= build
MAKE			= make

INCLUDES		= -I$(ROOT)/include
INCLUDES		+= -I$(ROOT)/lib/c/include
INCLUDES		+= -I$(ROOT)/arch/arm/platform


CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy


CPPFLAGS	= $(CONFIG_FLAG) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall
CPPFLAGS	+= -D__CONFIG_MUTEX__
#CPPFLAGS	+= -D__CONFIG_SMP__
