#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_BT_SmartBridge_Manufacturing_Test

# Enable Bluetooth Manufacturing Test Support
BT_MODE := MFGTEST

# Disable default STDIO. Manufacturing Test uses raw UART
GLOBAL_DEFINES += WICED_DISABLE_STDIO

# Set Bluetooth UART RX FIFO size to large enough number
GLOBAL_DEFINES += BT_BUS_RX_FIFO_SIZE=512

$(NAME)_SOURCES := bt_smartbridge_mfg_test.c

$(NAME)_COMPONENTS += bluetooth 

# Continuous Integration Test Definition
ifdef CI_TEST
ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM9WCDPLUS114))
VALID_PLATFORM := 1
else
VALID_PLATFORM :=
endif
else
ifneq ($(PLATFORM),$(filter $(PLATFORM), BCM9WCDPLUS114))
$(error The BT SmartBridge application only works with BCM9WCDPLUS114 platform)
endif
endif
