#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Bluetooth_RFCOMM_Client

$(NAME)_SOURCES := bt_rfcomm_client.c

$(NAME)_COMPONENTS := bluetooth \
					  bluetooth/protocols/RFCOMM

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