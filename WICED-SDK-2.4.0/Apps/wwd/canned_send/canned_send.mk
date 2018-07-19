#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Canned_Send_$(RTOS)

$(NAME)_SOURCES  := $(RTOS)_canned_send.c
$(NAME)_INCLUDES :=
$(NAME)_DEFINES  :=

# Disable watchdog for all WWD apps
GLOBAL_DEFINES += WICED_DISABLE_WATCHDOG

#To enable printing compile as a debug image. Note that printing requires a much larger stack.
ifneq ($(BUILD_TYPE),debug)
NO_STDIO := 1
NoOS_START_STACK := 700
else
#Large stack needed for printf in debug mode
NoOS_START_STACK := 4000
endif


ifdef CI_TEST
VALID_OSNS := NoOS-NoNS
VALID_NETWORK := NoNS
VALID_PLATFORM :=1
else

ifneq ($(NETWORK),NoNS)
$(error Canned_Send application only works with NoNS network-interface! )
endif
ifneq ($(RTOS),NoOS)
$(error Canned_Send application only works with NoOS rtos-interface! )
endif

endif