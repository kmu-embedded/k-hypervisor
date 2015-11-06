# TODO: This file will be generated automatically.
# Author has to know the difference among '?=', '=', '+=' and ':='
#
CROSS_COMPILE		?= arm-linux-gnueabihf-
ARCH				?= arm
CPU					?= cortex-a15

DEBUG_FLAG			= -g
CPPFLAGS			= -mcpu=$(CPU) -m$(ARCH) $(DEBUG_FLAGS)

MACH				?= rtsm
PROJECT				?= khypervisor
TARGET				?= khypervisor-$(MACH)
