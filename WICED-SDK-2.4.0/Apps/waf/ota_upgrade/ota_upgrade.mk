#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_OTA_Upgrade_App_

$(NAME)_SOURCES := content.c\
                   $(RTOS)_$(NETWORK)_ota_upgrade.c \
                   bsd-base64.c


LWIP_NUM_PACKET_BUFFERS_IN_POOL := 3

FreeRTOS_START_STACK := 600
ThreadX_START_STACK  := 600

GLOBAL_DEFINES := OTA_UPGRADE WEB_SERVER_NO_PRINT
# kaizen 20130408
GLOBAL_DEFINES += MAC_ADDRESS_SET_BY_HOST

GLOBAL_INCLUDES := .  $(RTOS)_$(NETWORK)/

NETX_ADD_DNS := 1

NO_WIFI_FIRMWARE := YES

ifeq ($(TOOLCHAIN_NAME),IAR)
#add optimization by size flag there
else
GLOBAL_CFLAGS := -Os
endif
APP_WWD_ONLY := 1

$(NAME)_COMPONENTS := waf/ota_upgrade/web_server \
                      waf/ota_upgrade/dhcp_server \
                      waf/ota_upgrade/dns_server

PROCESSORS_WITHOUT_ENOUGH_RAM := stm32f1x 

ifdef CI_TEST
VALID_OSNS := ThreadX-NetX_Duo
VALID_BUILD_TYPE := release
VALID_PLATFORM :=$(filter-out $(PROCESSORS_WITHOUT_ENOUGH_RAM),$(HOST_MICRO))
else
ifneq ($(RTOS)-$(NETWORK),ThreadX-NetX_Duo)
$(error waf.ota_upgrade only works with ThreadX-NetX_Duo combination!, not $(RTOS)-$(NETWORK) )
endif
ifneq ($(BUILD_TYPE),release)
$(error waf.ota_upgrade only works in release mode (not debug) )
endif

ifeq ($(filter-out $(PROCESSORS_WITHOUT_ENOUGH_RAM),$(HOST_MICRO)),)
$(error The selected platform does not have enough RAM for waf.ota_upgrade )
endif
endif