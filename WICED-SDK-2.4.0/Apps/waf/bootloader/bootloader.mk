#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_WICED_Bootloader_$(PLATFORM)

# MikeJ: Added ioutil.c, flash_if.c, ymodem.c
$(NAME)_SOURCES := bootloader.c \
                   ioutil.c \
                   flash_if.c \
                   ymodem.c

NoOS_START_STACK := 4000

APP_WWD_ONLY := 1
# MikeJ: Added WICED_DISABLE_STDIO
GLOBAL_DEFINES := WICED_NO_WIFI \
                  WICED_DISABLE_STDIO

GLOBAL_INCLUDES += .


ifdef CI_TEST
VALID_OSNS := NoOS-NoNS
VALID_BUILD_TYPE := release
VALID_PLATFORM :=1
else
ifneq ($(NETWORK),NoNS)
$(error Bootloader only works with NoNS network-interface! )
endif

ifneq ($(RTOS),NoOS)
$(error Bootloader only works with NoOS rtos-interface! )
endif
endif
