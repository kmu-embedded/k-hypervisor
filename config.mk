# TODO: This file will be generated automatically.
# Author has to know the difference among '?=', '=', '+=' and ':='
#
CROSS_COMPILE		?= arm-linux-gnueabihf-
PROCESSOR			?= cortex-a15

DEBUG_FLAG			= -g
CPPFLAGS			= -mcpu=$(PROCESSOR) -marm $(DEBUG_FLAGS)

MACH				?= rtsm
PROJECT				?= khypervisor
TARGET				?= khypervisor-$(MACH)

CURDIR				?= $(PWD)
ARCH				?= $(CURDIR)/arch/arm
ARCH_VERSION		?= $(ARCH)/v7
LIB					?= $(CURDIR)/lib
DRIVER				?= $(CURDIR)/drivers
