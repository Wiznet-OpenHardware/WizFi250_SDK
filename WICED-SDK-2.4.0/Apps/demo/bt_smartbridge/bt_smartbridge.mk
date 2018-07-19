#
# Copyright 2013, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_BT_SmartBridge

# Change to 1 to enable Apple MFi iAP
USE_APPLE_MFI_IAP := 0

$(NAME)_SOURCES := bt_smartbridge.c

$(NAME)_COMPONENTS += daemons/http_server \
                      protocols/sntp \
                      daemons/gedday \
                      bluetooth \
                      bluetooth/SmartBridge

$(NAME)_RESOURCES  += apps/bt_smartbridge/smartbridge_report.html \
                      apps/bt_smartbridge/data.html \
                      images/brcmlogo.png \
                      images/brcmlogo_line.png \
                      images/favicon.ico \
                      scripts/general_ajax_script.js \
                      scripts/wpad.dat

GLOBAL_DEFINES := USE_SELF_SIGNED_TLS_CERT

APPLICATION_DCT := bt_smartbridge_dct.c

# Wi-Fi credentials are either obtained by running iAP or from DCT 
ifeq (1,$(USE_APPLE_MFI_IAP))
$(NAME)_DEFINES    += USE_APPLE_MFI_IAP
$(NAME)_SOURCES    += bt_smartbridge_iap.c
$(NAME)_COMPONENTS += mfi/iap_server
else
WIFI_CONFIG_DCT_H  := wifi_config_dct.h
endif

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
